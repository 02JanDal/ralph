#include <QCoreApplication>
#include <QCommandLineParser>
#include <QDir>
#include <QDebug>
#include <iostream>

#include "Functions.h"
#include "Functional.h"
#include "CommandLineParser.h"
#include "config.h"

int main(int argc, char **argv) {
	QCoreApplication app(argc, argv);
	app.setApplicationName("ralph");
	app.setApplicationVersion(Ralph_CLIENT_VERSION);
	app.setOrganizationName("02JanDal");

	using namespace Ralph::Common::CommandLine;
	using Ralph::Client::State;

	State state;

	Parser cli;
	cli.setDescription("Package manager for C++");
	cli
			.addHelpCommand()
			.addHelpOption()
			.addVersionCommand()
			.addVersionOption()
			.add(Command("package", "Low-level commands for package management")
				 .add(Command("install", "Install the specified packages")
					  .then(state, &State::installPackage))
				 .add(Command("remove", "Remove the specified packages")
					  .then(state, &State::removePackage))
				 .add(Command("check", "Check that the package is available, but does not install it if not.")
					  .then(state, &State::checkPackage))
				 .add(PositionalArgument("packages", "The packages to install/update/remove").setMulti(true)))
			.add(Command("project", "High-level commands for package management")
				 .add(Option({"directory", "d"}, "DIR")
					  .setArgumentRequired(true)
					  .setDescription("Directory to look for ralph.json in")
					  .applyTo(state, &State::setDir))
				 .add(Command("verify", "Verifies if the structure of a ralph.json file is valid")
					  .then(state, &State::verifyProject))
				 .add(Command("new", "Creates a new project, including ralph.json, build system files and version control bootstraping")
					  .add(Option({"build-system", "bs"}, "BUILDSYSTEM")
						   .setArgumentRequired(true)
						   .setDefaultValue("cmake").setAllowedValues({"cmake"})
						   .setDescription("Which build system files to generate. Leave empty for none."))
					  .add(Option({"version-control-system", "vcs"}, "VCS")
						   .setArgumentRequired(true)
						   .setDefaultValue("git").setAllowedValues({"git"})
						   .setDescription("Which version control system to bootstrap for. Leave empty for none."))
					  .add(PositionalArgument("name", "The package name of the new project"))
					  .then(state, &State::newProject)))
			.add(Command("sources", "Manage package sources")
				 .add(Command("add", "Adds a new package source")
					  .then(state, &State::addSource))
				 .add(Command("remove", "removes an existing package source")
					  .then(state, &State::removeSource))
				 .add(Command("update", "Updates existing package sources")
					  .add(PositionalArgument("sources", "The names of the sources to update, leave empty for all").setMulti(true).setOptional(true))
					  .then(state, &State::updateSources)))
			.addCommandAlias("install", "package install")
			.addCommandAlias("remove", "package remove")
			.addCommandAlias("check", "package check")
			.addCommandAlias("verify", "project verify")
			.addCommandAlias("new", "project new")
			.addCommandAlias("update", "sources update");

	return cli.process(app);
}
