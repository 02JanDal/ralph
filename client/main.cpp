#include <QCoreApplication>
#include <QCommandLineParser>
#include <QDir>
#include <QDebug>
#include <iostream>

#include "Functions.h"
#include "Functional.h"

int main(int argc, char **argv) {
	QCoreApplication app(argc, argv);
	app.setApplicationName("ralph");
	app.setApplicationVersion("0.1");
	app.setOrganizationName("02JanDal");

	QCommandLineParser parser;
	parser.addHelpOption();
	parser.setApplicationDescription("Package manager for C++");
	parser.addOption(QCommandLineOption({"d", "directory"}, "Directory to look for ralph.json in, implies --project if no other option is given", "DIR", QDir::currentPath()));
	parser.addOption(QCommandLineOption({"p", "project"}, "Run in project mode"));
	parser.addOption(QCommandLineOption({"i", "install"}, "Install the specified package. Implied if no other action is given."));
	parser.addOption(QCommandLineOption({"r", "remove"}, "Remove the specified package"));
	parser.addOption(QCommandLineOption({"c", "check"}, "Check that the package is available, but does not install it if not."));
	parser.addOption(QCommandLineOption({"v", "version"}, "Specific package version or package version requirement"));
	parser.addOption(QCommandLineOption({"u", "update"}, "Updates the local package database"));
	parser.addPositionalArgument("PACKAGE", "The package to perform the specified action on.", "PACKAGE");

	parser.process(app);

	Ralph::Client::State state;
	if (parser.isSet("directory")) {
		state.setDir(parser.value("directory"));
	}

	if (parser.isSet("update")) {
		const int res = state.updatePackageDB();
		if (res != 0) {
			return res;
		}
	}

	if (parser.isSet("remove") || parser.isSet("install") || parser.isSet("check")) {
		const QString package = parser.positionalArguments().first();
		if (parser.isSet("remove") && parser.isSet("install") && parser.isSet("check")) {
			std::cerr << "--remove and --install are mutally exlusive.\n";
			return 1;
		} else if (parser.isSet("remove")) {
			return state.removePackage(package, parser.value("version"));
		} else if (parser.isSet("install")) {
			return state.installPackage(package, parser.value("version"));
		} else {
			return state.checkPackage(package, parser.value("version"));
		}
	} else if (parser.isSet("project") || parser.isSet("directory")) {
		return state.installForProject();
	} else {
		return 0;
	}
}
