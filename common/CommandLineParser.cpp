/* Copyright 2016 Jan Dalheimer <jan@dalheimer.de>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "CommandLineParser.h"

#include <QStringList>
#include <QCoreApplication>
#include <QRegularExpression>
#include <QDebug>

#include <iostream>

#include "TermUtil.h"

namespace Ralph {
namespace Common {
namespace CommandLine {

class CommandLineException : public Exception
{
public:
	explicit CommandLineException(const QString &message, const QVector<QString> &commandChain)
		: Exception(message), m_commandChain(commandChain) {}
	CommandLineException(const CommandLineException &) = default;
	virtual ~CommandLineException();

	QVector<QString> commandChain() const { return m_commandChain; }

private:
	QVector<QString> m_commandChain;
};

#define DEC_EXCEPTION(name) \
	QT_WARNING_PUSH \
	QT_WARNING_DISABLE_GCC("-Wweak-vtables") \
	class name##Exception : public CommandLineException { public: using CommandLineException::CommandLineException; } \
	QT_WARNING_POP
DEC_EXCEPTION(ToManyPositionals);
DEC_EXCEPTION(UnknownOption);
DEC_EXCEPTION(UnexpectedArgument);
DEC_EXCEPTION(MissingRequiredArgument);
DEC_EXCEPTION(MalformedOption);
DEC_EXCEPTION(MissingPositionalArgument);
DEC_EXCEPTION(InvalidOptionValue);
DEC_EXCEPTION(RepeatedOption);
DEC_EXCEPTION(Build);

int Parser::process(int argc, char **argv)
{
	QStringList list;
	for (int i = 0; i < argc; ++i) {
		list.append(QString::fromLocal8Bit(argv[0]));
	}
	return process(list);
}
int Parser::process(const QCoreApplication &app)
{
	setVersion(app.applicationVersion());
	setName(app.applicationName());
	return process(app.arguments());
}
int Parser::process(const QStringList &arguments)
{
	try {
		const Result result = parse(arguments);
		handle(result);
		if (result.options().isEmpty() && result.arguments().isEmpty() && result.commandChain().size() == 1) {
			printHelp();
		}
		return 0;
	} catch (BuildException &e) {
		std::cerr << e.what() << '\n'
				  << "This is a logic error in the program. Please report it to the developer.\n";
		return -1;
	} catch (CommandLineException &e) {
		std::cerr << Term::fg(Term::Red, e.what()) << "\n\n";
		printHelp(e.commandChain());
	} catch (Exception &e) {
		std::cerr << Term::fg(Term::Red, e.what()) << '\n';
		return -1;
	}
}

Parser &Parser::addVersionCommand()
{
	add(Command("version", "Show the version of this program").then([this]() { printVersion(); }));
	return *this;
}
Parser &Parser::addVersionOption()
{
	add(Option({"version", "v"})
		.setDescription("Show the version of this program")
		.setEarlyExit(true)
		.then([this]() { printVersion(); }));
	return *this;
}
Parser &Parser::addHelpCommand()
{
	add(Command("help", "Shows help for a given command")
		.add(PositionalArgument("subcommand", "The subcommand to show help for").setMulti(true).setOptional(true))
		.then([this](const Result &r) { printHelp(r.commandChain().mid(0, r.commandChain().size() - 1) + r.argumentMulti("subcommand")); }));
	return *this;
}
Parser &Parser::addHelpOption()
{
	add(Option({"help", "h"})
		.setDescription("Show help for the given command")
		.setEarlyExit(true)
		.then([this](const Result &r) { printHelp(r.commandChain()); }));
	return *this;
}

void Parser::printVersion()
{
	std::cout << name() << " - " << version() << '\n';
	std::exit(0);
}

static bool compareCommands(const Command &a, const Command &b)
{
	return a.name() < b.name();
}
QString formatPositionalArgument(const PositionalArgument &arg)
{
	QString result;
	if (arg.isOptional()) {
		result += '[';
	}
	result += '<' + arg.name();
	if (arg.isMulti()) {
		result += "...";
	}
	result += '>';
	if (arg.isOptional()) {
		result += ']';
	}
	return result;
}
QVector<QString> formatPositionalArguments(const QVector<PositionalArgument> &args)
{
	return Functional::map(args, &formatPositionalArgument);
}
static void printUsageFor(const QStringList &parents, const bool hasOptions, const QVector<QString> &positionals, const Command &command, const int maxWidth)
{
	QVector<QString> posArgs = positionals + formatPositionalArguments(command.arguments());
	if (!parents.isEmpty()) {
		std::cout << "    " << parents.join(' ');
		if (hasOptions) {
			std::cout << " [OPTIONS]";
		}

		if (!posArgs.isEmpty()) {
			std::cout << ' ' << posArgs.toList().join(' ');
		}
		std::cout << '\n';
	}

	for (const Command &sub : Functional::collection(command.subcommands().values()).sort(&compareCommands).get()) {
		if (!sub.isHidden()) {
			printUsageFor(parents + QStringList(sub.name()), hasOptions || !command.options().isEmpty(), posArgs, sub, maxWidth);
		}
	}
}
static void printSubcommandsTable(const QVector<Command> &commands, const int maxWidth)
{
	const QVector<QVector<QString>> rows = Functional::collection(commands)
			.filter([](const Command &command) { return !command.isHidden(); })
			.sort(&compareCommands)
			.map([](const Command &command) { return QVector<QString>({command.name(), "-", command.summary()}); });
	std::cout << "    " << Term::table(rows, {10, 1, 10}, maxWidth, 4) << '\n';
}
static void printOptionsTable(const QVector<Option> &options, const int maxWidth)
{
	const QVector<QVector<QString>> rows = Functional::map(options, [](const Option &option)
	{
		const QList<QString> syntax = Functional::map(option.names(), [option](const QString &name)
		{
			QString variant = QString('-');
			if (name.size() > 1) {
				variant += '-';
			}
			variant += name;
			if (option.hasArgument()) {
				if (option.isArgumentRequired()) {
					variant += "=<" + option.argument() + '>';
				} else  {
					variant += "[=<" + option.argument() + ">]";
				}
			}
			return variant;
		});

		QList<QString> help = QList<QString>() << option.description();
		if (option.hasArgument()) {
			if (!option.defaultValue().isNull()) {
				help.append(Term::style(Term::Bold, "Default: ") + option.defaultValue());
			}
			if (!option.allowedValues().isEmpty()) {
				help.append(Term::style(Term::Bold, "Allowed: ") + option.allowedValues().toList().join(", "));
			}
		}

		return QVector<QString>({syntax.join(", "), help.join('\n')});
	});
	std::cout << "    " << Term::table(rows, {1, 1}, maxWidth, 4) << '\n';
}
static void printArgumentsTable(const QVector<PositionalArgument> &arguments, const int maxWidth)
{
	const QVector<QVector<QString>> rows = Functional::map(arguments, [](const PositionalArgument &argument)
	{
		const QString syntax = formatPositionalArgument(argument);
		return QVector<QString>({syntax, argument.description()});
	});
	std::cout << "    " << Term::table(rows, {1, 1}, maxWidth, 4) << '\n';
}
void Parser::printHelp(const QVector<QString> &commands)
{
	const int maxWidth = Term::currentWidth() != 0 ? Term::currentWidth() : 120;

	QVector<QString> chain = commands.isEmpty() ? QVector<QString>({name()}) : commands;
	QVector<Option> options;
	QVector<PositionalArgument> positionals;
	Command command = Command(QString()).add(*this);
	for (const QString &cmd : chain) {
		command = command.subcommands().value(cmd);
		options.append(command.options());
		positionals.append(command.arguments());
	}

	using namespace Term;

	std::cout << name() << ' ' << version() << '\n'
			  << '\n'
			  << style(Bold, "Usage:") << '\n';

	printUsageFor(chain.toList(), !options.isEmpty(), {}, command, maxWidth);

	if (!command.subcommands().isEmpty()) {
		std::cout << '\n'
				  << style(Bold, "Subcommands:") << '\n';
		printSubcommandsTable(command.subcommands().values().toVector(), maxWidth);
	}
	if (!options.isEmpty()) {
		std::cout << '\n'
				  << style(Bold, "Options:") << '\n';
		printOptionsTable(options, maxWidth);
	}
	if (!positionals.isEmpty()) {
		std::cout << '\n'
				  << style(Bold, "Arguments:") << '\n';
		printArgumentsTable(positionals, maxWidth);
	}
	if (!command.description().isEmpty()) {
		std::cout << '\n'
				  << style(Bold, "Description:") << '\n';
		std::cout << "    " << wrap(command.description(), maxWidth, 4) << '\n';
	}

	std::exit(0);
}

Result Parser::parse(const QStringList &arguments) const
{
	static const QRegularExpression doubleOptionExpression("^--(?<name>[A-Za-z0-9-\\.]+)(?<valuecont>=(?<value>.*))?$");
	static const QRegularExpression singleOptionExpression("^-(?<names>[A-Za-z0-9]+)(?<valuecont>=(?<value>.*))?$");

	struct {
		QHash<QString, QVector<QString>> options;
		QHash<QString, QVector<QString>> arguments;
		QVector<QString> commandChain;
	} result;

	Command currentCommand;
	QHash<QString, Option> options;
	QVector<PositionalArgument> positionals;
	bool haveStartedPositionals = false; // after the first positional we don't allow any more subcommands

	auto nextCommand = [&currentCommand, &options, &positionals, &result](const Command &command)
	{
		for (const Option &option : command.options()) {
			for (const QString &name : option.names()) {
				options.insert(name, option);
			}
		}
		positionals.append(command.arguments());
		checkPositionals(positionals, result.commandChain);
		currentCommand = command;
		result.commandChain.append(command.name());
	};
	auto resolveAlias = [&currentCommand, &nextCommand](const QVector<QString> &path)
	{
		for (const QString &cmd : path) {
			nextCommand(currentCommand.subcommands().value(cmd));
		}
	};
	auto handleOption = [&result, &options](const QString &name, const QString &value, const bool hasValue, QStringListIterator *it)
	{
		if (!options.contains(name)) {
			throw UnknownOptionException("Unknown option: --" + name, result.commandChain);
		}
		const Option &option = options.value(name);
		if (hasValue && !option.hasArgument()) {
			throw UnexpectedArgumentException("Didn't expect an argument in " + (it ? it->peekPrevious() : name), result.commandChain);
		} else if (!hasValue && option.hasArgument() && option.isArgumentRequired()) {
			if (it && it->hasNext() && !it->peekNext().startsWith('-')) {
				result.options[option.names().first()].append(it->next());
				return;
			} else {
				throw MissingRequiredArgumentException("Missing required argument to -%1%2" % QString(name.size() > 1 ?  "-" : "") % name, result.commandChain);
			}
		}
		if (result.options.contains(option.names().first()) && !option.doesAllowMultiple()) {
			throw RepeatedOptionException("Received option -%1%2 multiple times, only once is allowed" % QString(name.size() > 1 ? "-" : "") % name, result.commandChain);
		}
		result.options[option.names().first()].append(value);
	};
	auto handleArguments = [&result, &haveStartedPositionals, &positionals](const QVector<QString> &args)
	{
		const int availablePositionals = Functional::collection(result.arguments.values()).mapSize().sum() + args.size();
		const bool haveMulti = !positionals.isEmpty() && positionals.last().isMulti();
		if (availablePositionals > positionals.size() && !haveMulti) {
			throw ToManyPositionalsException(QString("Expected no more than %1 positional arguments, got %2") % positionals.size() % availablePositionals,
											 result.commandChain);
		}
		for (const QString &arg : args) {
			if (positionals.size() == result.arguments.size()) {
				result.arguments[positionals.last().name()].append(arg);
			} else {
				result.arguments.insert(positionals.at(result.arguments.size()).name(), {arg});
			}
		}
		haveStartedPositionals = true;
	};

	nextCommand(*this);

	QStringListIterator it(arguments);
	it.next(); // application name
	while (it.hasNext()) {
		const QString item = it.next();
		if (!haveStartedPositionals && currentCommand.subcommands().contains(item)) {
			nextCommand(currentCommand.subcommands().value(item));
		} else if (!haveStartedPositionals && currentCommand.commandAliases().contains(item)) {
			resolveAlias(currentCommand.commandAliases().value(item));
		} else if (item == "--") {
			// everything after a -- needs to be positional arguments
			QVector<QString> remaining;
			while (it.hasNext()) {
				remaining.append(it.next());
			}
			handleArguments(remaining);
		} else if (item.startsWith("--")) {
			const QRegularExpressionMatch match = doubleOptionExpression.match(item);
			if (!match.hasMatch()) {
				throw MalformedOptionException("Malformed option: " + item, result.commandChain);
			}
			const QString name = match.captured("name");
			const QString value = match.captured("value");
			const bool hasValue = !match.captured("valuecont").isEmpty();
			handleOption(name, value, hasValue, &it);
		} else if (item.startsWith('-')) {
			const QRegularExpressionMatch match = singleOptionExpression.match(item);
			if (!match.hasMatch()) {
				throw MalformedOptionException("Malformed option: " + item, result.commandChain);
			}
			QString items = match.captured("names");
			const QString last = items.at(items.size() - 1);
			items = items.remove(items.size() - 1, 1);
			for (const QChar &option : items) {
				handleOption(option, QString(), false, nullptr);
			}
			handleOption(last, match.captured("value"), !match.captured("valuecont").isEmpty(), &it);
		} else {
			handleArguments({item});
		}
	}

	for (const Option &opt : options) {
		if (!result.options.contains(opt.names().first()) && opt.hasArgument() && !opt.defaultValue().isNull()) {
			result.options[opt.names().first()].append(opt.defaultValue());
		}
	}

	return Result(result.options, result.arguments, result.commandChain, options, positionals);
}

void Parser::handle(const Result &result) const
{
	for (const QString &option : result.options().keys()) {
		const Option &opt = result.possibleOptions().value(option);
		if (opt.isEarlyExit()) {
			opt.call(result);
		}
	}

	for (const PositionalArgument &arg : result.possiblePositionals()) {
		if ((!result.hasArgument(arg.name()) || result.argument(arg.name()).isEmpty()) && !arg.isOptional()) {
			throw MissingPositionalArgumentException(QString("Missing required positional argument '%1'").arg(arg.name()), result.commandChain());
		}
	}

	for (const QString &option : result.options().keys()) {
		const Option &opt = result.possibleOptions().value(option);
		for (const QString &value : result.values(option)) {
			if (!opt.allowedValues().isEmpty() && !opt.allowedValues().contains(value)) {
				throw InvalidOptionValueException(QString("The value to -%1%2 is not allowed; valid values: %3")
												  .arg(option.size() == 1 ? "-" : "")
												  .arg(option)
												  .arg(opt.allowedValues().toList().join(", ")),
												  result.commandChain());
			}
		}
		if (!opt.isEarlyExit()) {
			opt.call(result);
		}
	}
	Command cmd = Command().add(*this); // the first item in the commandChain will be the Parser, so we need to make sure the initial item has it as a child
	for (const QString &command : result.commandChain()) {
		cmd = cmd.subcommands().value(command);
		cmd.call(result);
	}
}

template <>
bool Result::value<bool>(const QString &key) const
{
	const QString val = value(key);
	if (!m_possibleOptions.value(key).hasArgument()) {
		if (key.startsWith("no-") || key.startsWith("disable-")) {
			return !isSet(key);
		} else {
			return isSet(key);
		}
	} else if (val == "1" || val.toLower() == "on" || val.toLower() == "true") {
		return true;
	} else {
		return false;
	}
}

void Command::checkPositionals(const QVector<PositionalArgument> &arguments, const QVector<QString> &commandChain)
{
	if (arguments.size() > 1) {
		const QVector<PositionalArgument> exceptLast = arguments.mid(0, arguments.size()-1);

		// only the last may be multi
		for (const PositionalArgument &arg : exceptLast) {
			if (arg.isMulti()) {
				throw BuildException("Only the last positional argument may be multi", commandChain);
			}
		}
		// if the last is multi, no arguments before it may be optional
		if (arguments.last().isMulti()) {
			for (const PositionalArgument &arg : exceptLast) {
				if (arg.isOptional()) {
					throw BuildException("May now have optional positional argument before multi positional argument", commandChain);
				}
			}
		}
		// no more required allowed after the first optional
		bool haveHadOptional = false;
		for (const PositionalArgument &arg : arguments) {
			if (haveHadOptional && !arg.isOptional()) {
				throw BuildException("May not have required positional arguments after optional ones", commandChain);
			} else if (arg.isOptional()) {
				haveHadOptional = true;
			}
		}
	}
}

QString detail::valueOf(const Option &option, const Result &result)
{
	return result.value(option.names().first());
}

CommandLineException::~CommandLineException() {}

}
}
}
