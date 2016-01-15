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

#include <QCoreApplication>
#include <QCommandLineParser>
#include <QDir>
#include <QDebug>
#include <QResource>
#include <iostream>

#include "Functions.h"
#include "Functional.h"
#include "CMakeIntegration.h"
#include "CommandLineParser.h"
#include "config.h"

int main(int argc, char **argv) {
	QCoreApplication app(argc, argv);
	app.setApplicationName("ralph");
	app.setApplicationVersion(Ralph_CLIENT_VERSION);
	app.setOrganizationName("02JanDal");

	Q_INIT_RESOURCE(resources);

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
					  .add(PositionalArgument("packages", "The packages to install").setMulti(true))
					  .add(Option({"group", "g"}, "GROUP")
						   .setDescription("Which package group to install into")
						   .setArgumentRequired(true).setDefaultValue(QString()))
					  .add(Option({"config", "c"}, "KEY:VALUE")
						   .setDescription("Sets an option to be used during installation")
						   .setArgumentRequired(true).setAllowMultiple(true))
					  .then(state, &State::installPackage))
				 .add(Command("remove", "Remove the specified packages")
					  .add(PositionalArgument("packages", "The packages to remove").setMulti(true))
					  .add(Option({"group", "g"}, "GROUP")
						   .setDescription("Which package group to remove from")
						   .setArgumentRequired(true).setDefaultValue(QString()))
					  .then(state, &State::removePackage))
				 .add(Command("check", "Check that the package is available, but does not install it if not.")
					  .add(PositionalArgument("packages", "The packages to check").setMulti(true))
					  .add(Option({"group", "g"}, "GROUP")
						   .setDescription("Which package group to check in")
						   .setArgumentRequired(true).setDefaultValue(QString()))
					  .then(state, &State::checkPackage))
				 .add(Command("search", "Searches for a package")
					  .add(PositionalArgument("query", "Filter packages by this query."))
					  .then(state, &State::searchPackages)))
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
					  .then(state, &State::newProject))
				 .add(Command("install", "Installs the packages specified in ralph.json")
					  .add(Option({"config", "c"}, "KEY:VALUE")
						   .setDescription("Sets an option to be used during installation")
						   .setArgumentRequired(true).setAllowMultiple(true))
					  .add(Option({"group", "g"}, "GROUP")
						   .setDescription("Which group to install packages into")
						   .setArgumentRequired(true).setDefaultValue(QString()))
					  .add(Option("in-project")
						   .setDescription("Install packages inside the project database"))
					  .then(state, &State::installProject))
				 .add(Command("update", "Updates packages for a specified ralph.json")
					  .add(Option({"config", "c"}, "KEY:VALUE")
						   .setDescription("Sets an option to be used during installation")
						   .setArgumentRequired(true).setAllowMultiple(true))
					  .add(Option({"group", "g"}, "GROUP")
						   .setDescription("Which group to install packages into")
						   .setArgumentRequired(true).setDefaultValue(QString()))
					  .add(Option("in-project")
						   .setDescription("Install packages inside the project database"))
					  .add(PositionalArgument("packages", "The packages to update")
						   .setMulti(true))
					  .then(state, &State::updateProject)))
			.add(Command("sources", "Manage package sources")
				 .add(Command("add", "Adds a new package source")
					  .add(PositionalArgument("name", "The name of the new source"))
					  .add(PositionalArgument("url", "The source url of the new source"))
					  .then(state, &State::addSource))
				 .add(Command("remove", "removes an existing package source")
					  .add(PositionalArgument("name", "The name of the source to remove"))
					  .then(state, &State::removeSource))
				 .add(Command("update", "Updates existing package sources")
					  .add(PositionalArgument("names", "The names of the sources to update, leave empty for all").setMulti(true).setOptional(true))
					  .then(state, &State::updateSources))
				 .add(Command("list", "Lists the available sources")
					  .then(state, &State::listSources))
				 .add(Command("show", "Shows information about a source")
					  .add(PositionalArgument("name", "The name of the source to remove"))
					  .then(state, &State::showSource))
				 .add(Option({"database", "db"}, "DATABASE")
					  .setArgumentRequired(true)
					  .setDefaultValue("user").setAllowedValues({"system", "user"})
					  .setDescription("Which database to use")))
			.add(Command("integration", "Helpers for various integrations")
				 .setHidden()
				 .add(Command("cmake", "Helpers for CMake integration")
					  .add(Command("cmake_path", "Base path to the CMake files")
						   .then(&Ralph::Integration::CMake::cmakePath))
					  .add(Command("load", "Generate a file with information about all CMake packages from ralph.json")
						   .add(PositionalArgument("basedir", "Base directory path to generate files in"))
						   .add(Option({"config", "c"}, "CONFIG")
								.setDescription("CMake build type/configuration").setArgumentRequired(true))
						   .then(&Ralph::Integration::CMake::cmakeLoad))))
			.add(Command("info", "Shows various debugging information about Ralph")
				 .then(state, &State::info))
			.addCommandAlias("install", "package install")
			.addCommandAlias("remove", "package remove")
			.addCommandAlias("check", "package check")
			.addCommandAlias("search", "package search")
			.addCommandAlias("verify", "project verify")
			.addCommandAlias("new", "project new")
			.addCommandAlias("update", "sources update");

	return cli.process(app);
}
