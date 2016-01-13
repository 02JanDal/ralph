# Copyright (C) 2000 Jan Dalheimer <jan@dalheimer.de>
# This work is free. You can redistribute it and/or modify it under the
# terms of the Do What The Fuck You Want To Public License, Version 2,
# as published by Sam Hocevar. See http://www.wtfpl.net/ for more details.

find_package(PkgConfig QUIET)
pkg_search_module(PC_LIBGIT2 libgit2)

find_path(LIBGIT2_INCLUDE_DIRS NAMES git2.h HINTS ${PC_LIBGIT2_INCLUDEDIR} ${PC_LIBGIT2_INCLUDE_DIRS})
find_library(LIBGIT2_LIBRARY_PATH NAMES git2 HINTS ${PC_LIBGIT2_LIBDIR} ${PC_LIBGIT2_LIBRARY_DIRS})

add_library(libgit2 SHARED IMPORTED)
set_target_properties(libgit2 PROPERTIES
    IMPORTED_LOCATION ${LIBGIT2_LIBRARY_PATH}
    INCLUDE_DIRECTORIES ${LIBGIT2_INCLUDE_DIRS})
#target_include_directories(libgit2 PUBLIC ${LIBGIT2_INCLUDE_DIRS})

set(LIBGIT2_LIBRARIES libgit2)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(LibGit2 DEFAULT_MSG LIBGIT2_LIBRARIES LIBGIT2_INCLUDE_DIRS)

mark_as_advanced(PC_LIBGIT2_INCLUDEDIR PC_LIBGIT2_INCLUDE_DIRS PC_LIBGIT2_LIBDIR PC_LIBGIT2_LIBRARY_DIRS LIBGIT2_LIBRARY_PATH)
