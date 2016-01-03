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
    # includes the cmake file(s) with the actual functions
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
    # and includes the cmake file(s) with the actual content. Those will look in the
    # current directory (fallback to the project directory) for a file called ralph.json,
    # which should contain the required packages, and download and install any missing.
    # It will additionally call find_package on all of them for you.
    include(ralph-load)

# Usage: Raw

Install the dependencies specified in ralph.json:

    ralph --project --install
    ralph -pi

Install a given package (latest stable version):

    ralph --install RapidJSON
    ralph -i RapidJSON

Install a given package with version requirement:

    ralph --install -v