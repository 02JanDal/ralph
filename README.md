# Ralph - A package manager for C++

Features:

* Install and remove packages
* Package groups
* Metadata stored as plain JSON and distributed using a plain git repository
* Project support
* Cross-platform
* Pre-built binaries and from-source installations
* Per-project databases

This project is still in early development and is not production ready.

## Installation

    $ git clone https://github.com/02JanDal/ralph.git
    $ cd ralph
    $ mkdir build && cd build
    $ cmake ..
    $ cmake --build .

## Usage

Start by simply running the executable without any arguments and you'll get a list of all available commands:

    $ ralph
    ralph 0.1
    ...

If you want more information about a specific command you can either use the help flag or the help command:

    $ ralph package install --help
    ...
    $ ralph help package install
    ...

Some commands to get you started:

    $ ralph sources add official https://github.com/02JanDal/ralph-packages.git
    ...
    $ ralph search json
    ...
    RapidJSON
    ...
    $ ralph install rapidjson
    ...

This adds the official repository of packages to the default database (usually the user database), searches for and installs a JSON library.

You might also be interested in

    $ ralph project new myproject

Which creates a new project with a ralph metaadata file that can be used for dependency management.

## Alternatives

C++ has gone from not having a single to having way to many package/dependency managers. And this is yet another one. Other options I am aware of:

* [conan.io](https://conan.io/)
* [CPM](https://github.com/iauns/cpm)
* [Hunter](https://github.com/ruslo/hunter)
* [NuGet](https://www.nuget.org/)

Apart from [NIH](https://en.wikipedia.org/wiki/Not_invented_here) this project also fixes some shortcomings with these existing solutions, like not being cross-platform (NuGet), only source packages (CPM, Hunter), being tied to a specific build system (CPM, Hunter), or having a horrible codebase (conan.io) (if mine is better or not is debatable).

Additionally this project includes some features inspired by other package managers like package groups (gemsets from RVM), per-project databases (bundler) and distribution using git repositories (homebrew) that can not be found in the above.

## Contributing

Normal GitHub procedure:

1. Fork on Github
2. Clone your fork locally
3. Edit code (preferably in a branch)
4. Commit and push your changes
5. Open a PR (pull request) on Github

Try to keep the same coding style as used in the rest of the project.

## License

Copyright © 2016 Jan Dalheimer <jan@dalheimer.de>

Licensed under the Apache License, Version 2.0 (the "License"); you may not use this program except in compliance with the License. You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0.

Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License.