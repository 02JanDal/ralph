# Ideas

* Install packages both from source and pre-built binaries
* From source packages should be able to have compile time options that can be set by the user when installing
	* A `ralph.json` project file should also be able to pre-define options
	* The options should work like either simply search-and-replace or using a small scripting language
* Different local installation databases
	* **System:** In `/var` or similar, rarely used probably
	* **User:** Somewhere under `$HOME`. If no other options are given this place should be used
	* **Project:** Inside the project folder, probably in a folder called `vendor`, `3rdparty` or similar, next to `ralph.json`
		* `ralph.json` can only specify options for building if installing into the project database
	* When looking for installed packages, first check in Project, then User, then System
* Different package sources
	* `gitrepo`: A repository of metadata files that's stored in a git repo
	* `git`: A single package that's stored in a git repo. Instead of a version a tag/branch/commitish can be specified
	* `github`: As `git`, but automatically fills in the git URL given a Github user/repo pair
	* Possible future extension: More VCS types, like SVN, Mercurial etc. (probably not)
* Scripted installations
	* Secure
		* No giving an unknown script root access
	* Probably ChaiScript or Lua
	* All file system access jailed into the installation directory
	* Special user permission for anything else

# Usage

## Usage: CMake

**Should** not do any work during configure phase, but need to have package available to
get paths and such from it. This isn't possible without sacrificing some of the
flexibility gained from CMake being a scripting language though, so package download and
install will be done when running CMake.

### Example 1

	cmake_minimum_required(VERSION 3.2)

	project(example1)

	# ralph.cmake (can be included in project) checks to make sure ralph is installed and
	# includes the cmake file(s) with the actual content.
	include(ralph)

	# A call to ralph_find_package will ensure that the package is installed and available
	# in the local ralph repository before invoking CMake's find_package with the correct
	# paths. If the package is not available and RALPH_INSTALL is ON (the default) it will
	# be downloaded and installed.
	ralph_find_package(RapidJSON REQUIRED)
	# As above, but also verifies the version. Components result in checking for
	# Qt5::Core, Qt5::Network as packages.
	ralph_find_package(Qt5 REQUIRED VERSION 5.5 COMPONENTS Core Network)

### Example 2

	cmake_minimum_required(VERSION 3.2)

	project(example2)

	# ralph-load.cmake (can be included in project) checks to make sure ralph is installed
	# and includes the cmake file(s) with the actual content.
	# ralph_load() will look in the current directory (fallback to the project directory or
	# the root source directory) for ralph.json, which contains the required packages,
	# and download and install any missing. It will additionally call find_package on all
	# of them for you.
	include(ralph)
	ralph_load()

# Usage: Raw

Install the dependencies specified in ralph.json:

	ralph project install

Install a given package (latest stable version):

	ralph install RapidJSON

Install a given package with version requirement:

	ralph install RapidJSON@1.0.2
