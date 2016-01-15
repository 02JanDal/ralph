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

#pragma once

#include <QString>
#include <QHash>
#include <QVector>
#include <QSet>
#include <functional>

#include "Exception.h"
#include "Functional.h"
#include "Optional.h"
#include "CommandLineParser_p.h"

QT_BEGIN_NAMESPACE
class QCoreApplication;
QT_END_NAMESPACE

namespace Ralph {
namespace Common {
namespace CommandLine {

class Result;

class Option
{
public:
	explicit Option(const QString &name, const QString &argument = QString())
		: Option(QStringList() << name, argument) {}
	explicit Option(const QStringList &names, const QString &argument = QString())
		: m_names(names), m_argumentName(argument) {}
	Option() {}

	QList<QString> names() const { return m_names; }

	QString description() const { return m_description; }
	Option &setDescription(const QString &description) { m_description = description; return *this; }

	bool hasArgument() const { return !m_argumentName.isNull(); }
	QString argument() const { return m_argumentName; }
	Option &setArgument(const QString &argument) { m_argumentName = argument; return *this; }

	bool isArgumentRequired() const { return m_argumentRequired; }
	Option &setArgumentRequired(const bool required) { m_argumentRequired = required; return *this; }

	QString defaultValue() const { return m_defaultValue; }
	Option &setDefaultValue(const QString &defaultValue) { m_defaultValue = defaultValue; return *this; }

	bool isEarlyExit() const { return m_earlyExit; }
	Option &setEarlyExit(const bool earlyExit) { m_earlyExit = earlyExit; return *this; }

	bool doesAllowMultiple() const { return m_allowMultiple; }
	Option &setAllowMultiple(const bool allowMultiple) { m_allowMultiple = allowMultiple; return *this; }

	QSet<QString> allowedValues() const { return m_allowedValues; }
	Option &setAllowedValues(const QSet<QString> &values) { m_allowedValues = values; return *this; }

	template <typename T>
	Option &applyTo(T &value)
	{
		return then<T>([&value](const T &val) { value = val; });
	}
	template <typename Class, typename Member, typename = std::enable_if_t<std::is_member_object_pointer<Member>::value>>
	Option &applyTo(Class &object, Member &&member)
	{
		using Type = decltype(member);
		return then<Type>([&object, &member](const Type &val) { Functional::eval<void, Class, Type>(member, object, val); });
	}
	template <typename Class, typename Func>
	Option &applyTo(Class &object, Func &&func)
	{
		using Type = typename Functional::FunctionTraits<Func>::template Argument<0>::Type;
		return then([&object, &func](const Type &val) { Functional::eval<void, Class, Type>(func, object, val); });
	}

	template <typename Func>
	Option &then(Func &&func)
	{
		using Traits = Functional::FunctionTraits<Func>;
		m_callback = detail::callback1(std::forward<Func>(func), *this, detail::Value<Traits::arity>());
		return *this;
	}

	void call(const Result &value) const { if (m_callback) { m_callback(value); } }

private:
	QStringList m_names;
	QString m_description;
	QString m_argumentName;
	QString m_defaultValue;
	bool m_argumentRequired = true;
	bool m_earlyExit = false;
	bool m_allowMultiple = false;
	QSet<QString> m_allowedValues;
	std::function<void(const Result &)> m_callback;
};

class PositionalArgument
{
public:
	explicit PositionalArgument(const QString &name, const QString &description = QString())
		: m_name(name), m_description(description) {}
	PositionalArgument() {}

	QString name() const { return m_name; }
	QString description() const { return m_description; }

	bool isOptional() const { return m_optional; }
	PositionalArgument &setOptional(const bool optional) { m_optional = optional; return *this; }

	bool isMulti() const { return m_multi; }
	PositionalArgument &setMulti(const bool multi) { m_multi = multi; return *this; }

private:
	QString m_name;
	QString m_description;
	bool m_optional = false;
	bool m_multi = false;
};

class Command
{
public:
	explicit Command(const QString &name, const QString &description = QString())
		: m_name(name), m_description(description) {}
	Command() {}

	QString name() const { return m_name; }
	QString description() const { return m_description; }
	QString summary() const { return m_description.mid(0, m_description.indexOf('\n')); }

	QVector<Option> options() const { return m_options; }
	Command &setOptions(const QVector<Option> &options) { m_options = options; return *this; }
	Command &add(const Option &option) { m_options.append(option); return *this; }

	QVector<PositionalArgument> arguments() const { return m_arguments; }
	Command &setArguments(const QVector<PositionalArgument> &arguments) { m_arguments = arguments; checkPositionals(m_arguments); return *this; }
	Command &add(const PositionalArgument &argument) { m_arguments.append(argument); checkPositionals(m_arguments); return *this; }

	QHash<QString, Command> subcommands() const { return m_subcommands; }
	Command &setSubcommands(const QHash<QString, Command> &subcommands) { m_subcommands = subcommands; return *this; }
	Command &add(const Command &subcommand) { m_subcommands.insert(subcommand.name(), subcommand); return *this; }

	QHash<QString, QVector<QString>> commandAliases() const { return m_commandAliases; }
	Command &addCommandAlias(const QString &alias, const QString &commandPath) { return addCommandAlias(alias, commandPath.split(' ').toVector()); }
	Command &addCommandAlias(const QString &alias, const QVector<QString> &commandPath) { m_commandAliases.insert(alias, commandPath); return *this; }

	bool isHidden() const { return m_hidden; }
	Command &setHidden(const bool hidden = true) { m_hidden = hidden; return *this; }

	template <typename Class, typename Func>
	Command &then(Class &object, Func &&func)
	{
		Functional::static_if<Functional::FunctionTraits<Func>::arity == 0>([this, &object, &func](auto f)
		{
			using Type = typename Functional::FunctionTraits<Func>::ReturnType;
			then([f, &object, &func]() { Functional::eval<Type, Class>(f(func), object); });
		}).else_([this, &object, &func](auto f)
		{
			using Type = typename Functional::FunctionTraits<decltype(f(func))>::template Argument<0>::Type;
			then([&object, &func](Type value) { Functional::eval(std::forward<Func>(func), object, value); });
		});
		return *this;
	}

	template <typename Func>
	Command &then(Func &&func)
	{
		Functional::static_if<Functional::FunctionTraits<Func>::arity == 0>([this, func](auto f)
		{
			m_callback = [func, f](const Result &) { f(func)(); };
		}).else_([this, func](auto f)
		{
			m_callback = f(func);
		});
		return *this;
	}

	void call(const Result &result) { if (m_callback) { m_callback(result); } }

protected:
	void setName(const QString &name) { m_name = name; }
	static void checkPositionals(const QVector<PositionalArgument> &arguments, const QVector<QString> &commandChain = {});

private:
	friend class Parser;
	QString m_name;
	QString m_description;
	QVector<Option> m_options;
	QVector<PositionalArgument> m_arguments;
	QHash<QString, Command> m_subcommands;
	QHash<QString, QVector<QString>> m_commandAliases;
	bool m_hidden = false;

	std::function<void(const Result &)> m_callback;
};
inline Command &operator<<(Command &command, const Command &subcommand)
{
	return command.add(subcommand);
}
inline Command &operator<<(Command &command, const PositionalArgument &argument)
{
	return command.add(argument);
}
inline Command &operator<<(Command &command, const Option &option)
{
	return command.add(option);
}

class Result
{
public:
	explicit Result(const QHash<QString, QVector<QString>> &options,
					const QHash<QString, QVector<QString>> &arguments,
					const QVector<QString> &commandChain,
					const QHash<QString, Option> &possibleOptions,
					const QVector<PositionalArgument> &possiblePositionals)
		: m_options(options), m_arguments(arguments), m_commandChain(commandChain), m_possibleOptions(possibleOptions), m_possiblePositionals(possiblePositionals) {}

public: // options
	QHash<QString, QVector<QString>> options() const { return m_options; }
	QHash<QString, Option> possibleOptions() const { return m_possibleOptions; }

	bool isSet(const QString &key) const { return m_options.contains(key); }
	bool contains(const QString &key) const { return m_options.contains(key); }

	QString value(const QString &key) const { return m_options.value(key, QVector<QString>(1)).first(); }
	template <typename T>
	T value(const QString &key) const { return detail::convert(value(key), detail::Type<T>()); }

	QVector<QString> values(const QString &key) const { return m_options.value(key); }

public: // arguments
	QHash<QString, QVector<QString>> arguments() const { return m_arguments; }
	QVector<PositionalArgument> possiblePositionals() const { return m_possiblePositionals; }

	bool hasArgument(const QString &name) const { return m_arguments.contains(name); }

	QString argument(const QString &key) const { return m_arguments.value(key, QVector<QString>(1)).first(); }
	QVector<QString> argumentMulti(const QString &key) const { return m_arguments.value(key); }
	template <typename T>
	T argument(const QString &key) const { return detail::convert(argument(key), detail::Type<T>()); }

public: // other
	QVector<QString> commandChain() const { return m_commandChain; }

private:
	QHash<QString, QVector<QString>> m_options;
	QHash<QString, QVector<QString>> m_arguments;
	QVector<QString> m_commandChain;
	QHash<QString, Option> m_possibleOptions;
	QVector<PositionalArgument> m_possiblePositionals;
};
template <>
bool Result::value<bool>(const QString &key) const;

class Parser : public Command
{
public:
	explicit Parser() {}

	int process(int argc, char **argv);
	int process(const QCoreApplication &app);
	int process(const QStringList &arguments);

	QString version() const { return m_version; }
	Parser &setVersion(const QString &version) { m_version = version; return *this; }

	Parser &setDescription(const QString &description) { m_description = description; return *this; }

	Parser &addVersionCommand();
	Parser &addVersionOption();
	Parser &addHelpCommand();
	Parser &addHelpOption();

	Q_NORETURN void printVersion();
	Q_NORETURN void printHelp(const QVector<QString> &commands = QVector<QString>());

private:
	QString m_version;

	Result parse(const QStringList &arguments) const;
	void handle(const Result &result) const;
};

}
}
}
