#.rst:
# Findlibhybris
# -------
#
# Try to find libhybris on a Unix system.

#=============================================================================
# Copyright 2015 Martin Gräßlin <mgraesslin@kde.org>
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#
# 1. Redistributions of source code must retain the copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
# 3. The name of the author may not be used to endorse or promote products
#    derived from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
# IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
# OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
# IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
# INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
# NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
# THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#=============================================================================

if(CMAKE_VERSION VERSION_LESS 2.8.12)
    message(FATAL_ERROR "CMake 2.8.12 is required by Findlibhybris.cmake")
endif()
if(CMAKE_MINIMUM_REQUIRED_VERSION VERSION_LESS 2.8.12)
    message(AUTHOR_WARNING "Your project should require at least CMake 2.8.12 to use Findlibhybris.cmake")
endif()

if(NOT WIN32)
    # Use pkg-config to get the directories and then use these values
    # in the FIND_PATH() and FIND_LIBRARY() calls
    find_package(PkgConfig)
    pkg_check_modules(PKG_libhardware QUIET libhardware)
    pkg_check_modules(PKG_androidheaders QUIET android-headers)
    pkg_check_modules(PKG_hwcomposerwindow QUIET hwcomposer-egl)
    pkg_check_modules(PKG_hybriseglplatform QUIET hybris-egl-platform)
    pkg_check_modules(PKG_hybrissync QUIET libsync)
    pkg_check_modules(PKG_hybrisinputstack QUIET libis)

    set(libhardware_DEFINITIONS ${PKG_libhardware_CFLAGS_OTHER})
    set(libhardware_VERSION ${PKG_libhardware_VERSION})

    find_library(libhardware_LIBRARY
        NAMES
            libhardware.so
        HINTS
            ${PKG_libhardware_LIBRARY_DIRS}
    )
    find_path(libhardware_INCLUDE_DIR
        NAMES
            android-version.h
        HINTS
            ${PKG_androidheaders_INCLUDE_DIRS}
    )

    include(FindPackageHandleStandardArgs)
    find_package_handle_standard_args(libhardware
        FOUND_VAR
            libhardware_FOUND
        REQUIRED_VARS
            libhardware_LIBRARY
            libhardware_INCLUDE_DIR
        VERSION_VAR
            libhardware_VERSION
    )

    if(libhardware_FOUND AND NOT TARGET libhybris::libhardware)
        add_library(libhybris::libhardware UNKNOWN IMPORTED)
        set_target_properties(libhybris::libhardware PROPERTIES
            IMPORTED_LOCATION "${libhardware_LIBRARY}"
            INTERFACE_COMPILE_OPTIONS "${libhardware_DEFINITIONS}"
            INTERFACE_INCLUDE_DIRECTORIES "${libhardware_INCLUDE_DIR}"
        )
    endif()

    mark_as_advanced(libhardware_LIBRARY libhardware_INCLUDE_DIR)

    ##############################################
    # hwcomposerWindow
    ##############################################
    set(libhwcomposer_DEFINITIONS ${PKG_hwcomposerwindow_CFLAGS_OTHER})
    set(libhwcomposer_VERSION ${PKG_hwcomposerwindow_VERSION})

    find_library(libhwcomposer_LIBRARY
        NAMES
            libhybris-hwcomposerwindow.so
        HINTS
            ${PKG_hwcomposerwindow_LIBRARY_DIRS}
    )
    find_path(libhwcomposer_INCLUDE_DIR
        NAMES
            hwcomposer_window.h
        HINTS
            ${PKG_hwcomposerwindow_INCLUDE_DIRS}
    )

    include(FindPackageHandleStandardArgs)
    find_package_handle_standard_args(libhwcomposer
        FOUND_VAR
            libhwcomposer_FOUND
        REQUIRED_VARS
            libhwcomposer_LIBRARY
            libhwcomposer_INCLUDE_DIR
        VERSION_VAR
            libhwcomposer_VERSION
    )

    if(libhwcomposer_FOUND AND NOT TARGET libhybris::hwcomposer)
        add_library(libhybris::hwcomposer UNKNOWN IMPORTED)
        set_target_properties(libhybris::hwcomposer PROPERTIES
            IMPORTED_LOCATION "${libhwcomposer_LIBRARY}"
            INTERFACE_COMPILE_OPTIONS "${libhardware_DEFINITIONS}"
            INTERFACE_INCLUDE_DIRECTORIES "${libhwcomposer_INCLUDE_DIR}"
        )
    endif()

    mark_as_advanced(libhwcomposer_LIBRARY libhwcomposer_INCLUDE_DIR)

    ##############################################
    # hybriseglplatform
    ##############################################
    set(hybriseglplatform_DEFINITIONS ${PKG_hybriseglplatform_CFLAGS_OTHER})
    set(hybriseglplatform_VERSION ${PKG_hybriseglplatform_VERSION})

    find_library(hybriseglplatform_LIBRARY
        NAMES
            libhybris-eglplatformcommon.so
        HINTS
            ${PKG_hybriseglplatform_LIBRARY_DIRS}
    )
    find_path(hybriseglplatform_INCLUDE_DIR
        NAMES
            eglplatformcommon.h
        HINTS
            ${PKG_hybriseglplatform_INCLUDE_DIRS}
    )

    include(FindPackageHandleStandardArgs)
    find_package_handle_standard_args(hybriseglplatform
        FOUND_VAR
            hybriseglplatform_FOUND
        REQUIRED_VARS
            hybriseglplatform_LIBRARY
            hybriseglplatform_INCLUDE_DIR
        VERSION_VAR
            hybriseglplatform_VERSION
    )

    if(hybriseglplatform_FOUND AND NOT TARGET libhybris::hybriseglplatform)
        add_library(libhybris::hybriseglplatform UNKNOWN IMPORTED)
        set_target_properties(libhybris::hybriseglplatform PROPERTIES
            IMPORTED_LOCATION "${hybriseglplatform_LIBRARY}"
            INTERFACE_COMPILE_OPTIONS "${hybriseglplatform_DEFINITIONS}"
            INTERFACE_INCLUDE_DIRECTORIES "${hybriseglplatform_INCLUDE_DIR}"
        )
    endif()

    mark_as_advanced(hybriseglplatform_LIBRARY hybriseglplatform_INCLUDE_DIR)

    ##############################################
    # hybrissync
    ##############################################
    set(hybrissync_DEFINITIONS ${PKG_hybrissync_CFLAGS_OTHER})
    set(hybrissync_VERSION ${PKG_hybrissync_VERSION})

    find_library(hybrissync_LIBRARY
        NAMES
            libsync.so
        HINTS
            ${PKG_hybrissync_LIBRARY_DIRS}
    )

    include(FindPackageHandleStandardArgs)
    find_package_handle_standard_args(hybrissync
        FOUND_VAR
            hybrissync_FOUND
        REQUIRED_VARS
            hybrissync_LIBRARY
        VERSION_VAR
            hybrissync_VERSION
    )

    if(hybrissync_FOUND AND NOT TARGET libhybris::sync)
        add_library(libhybris::sync UNKNOWN IMPORTED)
        set_target_properties(libhybris::sync PROPERTIES
            IMPORTED_LOCATION "${hybrissync_LIBRARY}"
            INTERFACE_COMPILE_OPTIONS "${hybrissync_DEFINITIONS}"
        )
    endif()

    mark_as_advanced(hybrissync_LIBRARY)

    ##############################################
    # hybrisinputstack
    ##############################################
    set(hybrisinputstack_DEFINITIONS ${PKG_hybrisinputstack_CFLAGS_OTHER})
    set(hybrisinputstack_VERSION ${PKG_hybrisinputstack_VERSION})

    find_library(hybrisinputstack_LIBRARY
        NAMES
            libis.so
        HINTS
            ${PKG_hybrisinputstack_LIBRARY_DIRS}
    )

    include(FindPackageHandleStandardArgs)
    find_package_handle_standard_args(hybrisinputstack
        FOUND_VAR
            hybrisinputstack_FOUND
        REQUIRED_VARS
            hybrisinputstack_LIBRARY
        VERSION_VAR
            hybrisinputstack_VERSION
    )

    if(hybrisinputstack_FOUND AND NOT TARGET libhybris::inputstack)
        add_library(libhybris::inputstack UNKNOWN IMPORTED)
        set_target_properties(libhybris::inputstack PROPERTIES
            IMPORTED_LOCATION "${hybrisinputstack_LIBRARY}"
            INTERFACE_COMPILE_OPTIONS "${hybrisinputstack_DEFINITIONS}"
        )
    endif()

    mark_as_advanced(hybrisinputstack_LIBRARY)

    if(libhardware_FOUND AND libhwcomposer_FOUND AND hybriseglplatform_FOUND AND hybrissync_FOUND AND hybrisinputstack_FOUND)
        set(libhybris_FOUND TRUE)
    else()
        set(libhybris_FOUND FALSE)
    endif()

else()
    message(STATUS "Findlibhardware.cmake cannot find libhybris on Windows systems.")
    set(libhybris_FOUND FALSE)
endif()

include(FeatureSummary)
set_package_properties(libhybris PROPERTIES
    URL "https://github.com/libhybris/libhybris"
    DESCRIPTION "libhybris allows to run bionic-based HW adaptations in glibc systems."
)
