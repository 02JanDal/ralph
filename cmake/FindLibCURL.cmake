# Copyright (C) 2016 Jan Dalheimer <jan@dalheimer.de>
# This work is free. You can redistribute it and/or modify it under the
# terms of the Do What The Fuck You Want To Public License, Version 2,
# as published by Sam Hocevar. See http://www.wtfpl.net/ for more details.

find_package(CURL QUIET)

add_library(libcurl SHARED IMPORTED)
set_target_properties(libcurl PROPERTIES
    IMPORTED_LOCATION ${CURL_LIBRARIES}
    INCLUDE_DIRECTORIES ${CURL_INCLUDE_DIRS})

set(LIBCURL_LIBRARIES libcurl)
set(LIBCURL_INCLUDE_DIRS ${CURL_INCLUDE_DIRS})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(LibCURL DEFAULT_MSG LIBCURL_LIBRARIES LIBCURL_INCLUDE_DIRS)

mark_as_advanced(LIBCURL_INCLUDE_DIRS CURL_LIBRARIES)
