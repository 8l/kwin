project(KWIN)
set(PROJECT_VERSION "5.3.90")
set(PROJECT_VERSION_MAJOR 5)

cmake_minimum_required(VERSION 2.8.12 FATAL_ERROR)
set(QT_MIN_VERSION "5.4.0")
set(KF5_MIN_VERSION "5.8.0")

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules ${CMAKE_MODULE_PATH} )

find_package(ECM 0.0.11 REQUIRED NO_MODULE)

include(FeatureSummary)
include(WriteBasicConfigVersionFile)
include(GenerateExportHeader)

# where to look first for cmake modules, before ${CMAKE_ROOT}/Modules/ is checked
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR})

find_package(Qt5 ${QT_MIN_VERSION} CONFIG REQUIRED COMPONENTS
    Concurrent
    Core
    DBus
    Quick
    QuickWidgets
    Script
    UiTools
    Widgets
    X11Extras
)

find_package(Qt5Test ${QT_MIN_VERSION} CONFIG QUIET)
set_package_properties(Qt5Test PROPERTIES
         PURPOSE "Required for tests"
         TYPE OPTIONAL
         )
add_feature_info("Qt5Test" Qt5Test_FOUND "Required for building tests")
if (NOT Qt5Test_FOUND)
    set(BUILD_TESTING OFF CACHE BOOL "Build the testing tree.")
endif()

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings)

include(ECMInstallIcons)
include(ECMOptionalAddSubdirectory)

add_definitions(-DQT_DISABLE_DEPRECATED_BEFORE=0)

# require at least gcc 4.8
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    if ("${CMAKE_CXX_COMPILER_VERSION}" VERSION_LESS "4.8")
        message(SEND_ERROR "Version ${CMAKE_CXX_COMPILER_VERSION} of the ${CMAKE_CXX_COMPILER_ID} C++ compiler is not supported. Please use version 4.8 or later.")
    endif()
endif()

find_package(Qt5Multimedia QUIET)
set_package_properties(Qt5Multimedia PROPERTIES
         PURPOSE "Runtime-only dependency for effect video playback"
         TYPE RUNTIME
         )


# required frameworks by Core
find_package(KF5 ${KF5_MIN_VERSION} REQUIRED COMPONENTS
    Config
    ConfigWidgets
    CoreAddons
    Crash
    GlobalAccel
    I18n
    Init
    Notifications
    Service
    Plasma
    WidgetsAddons
    WindowSystem
    IconThemes
)
# required frameworks by config modules
find_package(KF5 ${KF5_MIN_VERSION} REQUIRED COMPONENTS
    Completion
    Declarative
    KCMUtils
    KIO
    NewStuff
    XmlGui
)

find_package(Threads)
set_package_properties(Threads PROPERTIES
         PURPOSE "Needed for VirtualTerminal support in KWin Wayland"
         TYPE REQUIRED
         )

# optional frameworks
find_package(KF5Activities ${KF5_MIN_VERSION} CONFIG)
set_package_properties(KF5Activities PROPERTIES
         PURPOSE "Enable building of KWin with kactivities support"
         TYPE OPTIONAL
         )
add_feature_info("KF5Activities" KF5Activities_FOUND "Enable building of KWin with kactivities support")

find_package(KF5DocTools ${KF5_MIN_VERSION} CONFIG)
set_package_properties(KF5DocTools PROPERTIES
         PURPOSE "Enable building documentation"
         TYPE OPTIONAL
         )
add_feature_info("KF5DocTools" KF5DocTools_FOUND "Enable building documentation")

find_package(KDecoration2 CONFIG REQUIRED)
find_package(KF5Wayland CONFIG)
set_package_properties(KF5Wayland PROPERTIES
                       TYPE OPTIONAL
                      )
add_feature_info("KF5Wayland" KF5Wayland_FOUND "Required for Wayland Compositor Information Module")

find_package(EGL)
set_package_properties(EGL PROPERTIES
                       TYPE RUNTIME
                       PURPOSE "Required to build KWin with EGL support"
                      )

find_package(epoxy)
set_package_properties(epoxy PROPERTIES DESCRIPTION "libepoxy"
                       URL "http://github.com/anholt/libepoxy"
                       TYPE REQUIRED
                       PURPOSE "OpenGL dispatch library"
                      )

find_package(Wayland 1.2 COMPONENTS Egl Cursor)
set_package_properties(Wayland PROPERTIES
                       TYPE OPTIONAL
                       PURPOSE "Required for building KWin with Wayland support"
                      )
add_feature_info("Wayland-Cursor" Wayland_Cursor_FOUND "Required for cursor support in Wayland backend of kwin_wayland")
add_feature_info("Wayland-EGL" Wayland_Egl_FOUND "Required for building the Wayland EGL compositing backend in KWin")

find_package(XKB 0.4.1)
set_package_properties(XKB PROPERTIES
                       TYPE OPTIONAL
                       PURPOSE "Required for building KWin with Wayland support"
                      )

find_package(Libinput 0.10)
set_package_properties(Libinput PROPERTIES TYPE OPTIONAL PURPOSE "Required for input handling on Wayland.")

find_package(UDev)
set_package_properties(UDev PROPERTIES  URL "http://www.freedesktop.org/software/systemd/libudev/"
                                        DESCRIPTION "Linux device library."
                                        TYPE OPTIONAL
                                        PURPOSE "Required for input handling on Wayland."
                        )
set(HAVE_INPUT FALSE)
if (Libinput_FOUND AND UDEV_FOUND)
    set(HAVE_INPUT TRUE)
endif()

find_package(Libdrm)
set_package_properties(Libdrm PROPERTIES TYPE OPTIONAL PURPOSE "Required for drm output on Wayland.")
set(HAVE_DRM FALSE)
if(Libdrm_FOUND AND UDEV_FOUND)
    set(HAVE_DRM TRUE)
endif()

find_package(gbm)
set_package_properties(gbm PROPERTIES TYPE OPTIONAL PURPOSE "Required for egl ouput of drm backend.")
set(HAVE_GBM FALSE)
if(HAVE_DRM AND gbm_FOUND)
    set(HAVE_GBM TRUE)
endif()

find_package(libhybris)
set_package_properties(libhybris PROPERTIES TYPE OPTIONAL PURPOSE "Required for libhybris backend")
set(HAVE_LIBHYBRIS ${libhybris_FOUND})

find_package(X11)
set_package_properties(X11 PROPERTIES DESCRIPTION "X11 libraries"
                        URL "http://www.x.org"
                        TYPE REQUIRED
                    )

# All the required XCB components
find_package(XCB
    REQUIRED COMPONENTS
        XCB
        XFIXES
        DAMAGE
        COMPOSITE
        SHAPE
        SYNC
        RENDER
        RANDR
        KEYSYMS
        IMAGE
        SHM
        GLX
    OPTIONAL_COMPONENTS
        ICCCM
        CURSOR
)
set_package_properties(XCB PROPERTIES TYPE REQUIRED)

# and the optional XCB dependencies
if (XCB_ICCCM_VERSION VERSION_LESS "0.4")
    set(XCB_ICCCM_FOUND FALSE)
endif()
add_feature_info("XCB-ICCCM" XCB_ICCCM_FOUND "Required for building test applications for KWin")
add_feature_info("XCB-CURSOR" XCB_CURSOR_FOUND "Required for XCursor support")

set(HAVE_XCB_SYNC TRUE)
if (XCB_VERSION VERSION_LESS "1.10")
    set(HAVE_XCB_SYNC FALSE)
endif()
add_feature_info("XCB-SYNC" HAVE_XCB_SYNC "XCB Sync version >= 1.10 required for synced window resizing")

find_package(X11_XCB)
set_package_properties(X11_XCB PROPERTIES
                       PURPOSE "Required for OpenGL support in X11 windowed backend of kwin_wayland"
                       TYPE OPTIONAL)

########### configure tests ###############
include(CMakeDependentOption)

option(KWIN_BUILD_DECORATIONS "Enable building of KWin decorations." ON)
option(KWIN_BUILD_KCMS "Enable building of KWin configuration modules." ON)
option(KWIN_BUILD_TABBOX "Enable building of KWin Tabbox functionality" ON)
option(KWIN_BUILD_XRENDER_COMPOSITING "Enable building of KWin with XRender Compositing support" ON)
cmake_dependent_option(KWIN_BUILD_ACTIVITIES "Enable building of KWin with kactivities support" ON "KF5Activities_FOUND" OFF)
option(KWIN_PLASMA_ACTIVE "Enable building KWin for Plasma Active." OFF)
option(KWIN_BUILD_EGL "Enables building the EGL backend in KWin" ON)

option(KWIN_BUILD_COVERAGE "Build KWin with gcov support" OFF)

if(KWIN_BUILD_COVERAGE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-arcs -ftest-coverage")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -lgcov")
endif()

# Binary name of KWin
set(KWIN_NAME "kwin")
set(KWIN_INTERNAL_NAME_X11 "kwin_x11")
set(KWIN_INTERNAL_NAME_WAYLAND "kwin_wayland")
set(KWIN_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})

if(KWIN_PLASMA_ACTIVE)
    set(KWIN_BUILD_DECORATIONS OFF)
    set(KWIN_BUILD_KCMS OFF)
    set(KWIN_BUILD_XRENDER_COMPOSITING OFF)
    set(KWIN_BUILD_WITH_OPENGLES ON)
    set(KWIN_NAME "kwinactive")
endif()

set(KWIN_BUILD_OPENGL FALSE)
set(KWIN_BUILD_OPENGLES FALSE)
if(${Qt5Gui_OPENGL_IMPLEMENTATION} STREQUAL "GL")
    set(KWIN_BUILD_OPENGL TRUE)
    message("Building KWin with OpenGL support")
endif()
if(${Qt5Gui_OPENGL_IMPLEMENTATION} STREQUAL "GLESv2")
    set(KWIN_BUILD_OPENGLES TRUE)
    message("Building KWin with OpenGL ES 2 support")
    add_definitions(-DKWIN_HAVE_OPENGLES)
endif()

if(NOT KWIN_BUILD_OPENGL AND NOT KWIN_BUILD_OPENGLES)
    message(FATAL_ERROR "KWin needs to be built against either OpenGL or OpenGL ES 2. Check your Qt config!")
endif()

# KWIN_HAVE_XRENDER_COMPOSITING - whether XRender-based compositing support is available: may be disabled
if( KWIN_BUILD_XRENDER_COMPOSITING )
    set( KWIN_HAVE_XRENDER_COMPOSITING 1 )
endif()

set(KWIN_HAVE_EGL ${KWIN_BUILD_EGL})

if (XKB_FOUND)
  include_directories(${XKB_INCLUDE_DIR})
endif()

include_directories(${epoxy_INCLUDE_DIR})

# for things that are also used by kwin libraries
configure_file(libkwineffects/kwinconfig.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/libkwineffects/kwinconfig.h )
# for kwin internal things
if(XKB_FOUND AND KF5Wayland_FOUND)
    set(HAVE_WAYLAND TRUE)
    set(HAVE_XKB ${XKB_FOUND})
    if (KWIN_HAVE_EGL)
        set(HAVE_WAYLAND_EGL ${Wayland_Egl_FOUND})
    else()
        set(HAVE_WAYLAND_EGL FALSE)
    endif()
    set(HAVE_WAYLAND_CURSOR ${Wayland_Cursor_FOUND})
    set(HAVE_X11_XCB ${X11_XCB_FOUND})
else()
    set(HAVE_WAYLAND FALSE)
    set(HAVE_XKB FALSE)
    set(HAVE_WAYLAND_EGL FALSE)
    set(HAVE_WAYLAND_CURSOR FALSE)
    set(HAVE_X11_XCB FALSE)
    set(HAVE_DRM FALSE)
endif()

if(XCB_CURSOR_FOUND)
    set(HAVE_XCB_CURSOR ${XCB_CURSOR_FOUND})
else()
    set(HAVE_XCB_CURSOR FALSE)
endif()

include(CheckIncludeFiles)
check_include_files(unistd.h HAVE_UNISTD_H)
check_include_files(malloc.h HAVE_MALLOC_H)
configure_file(config-kwin.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-kwin.h )


########### global ###############
set(kwin_effects_dbus_xml ${CMAKE_CURRENT_SOURCE_DIR}/org.kde.kwin.Effects.xml)

include_directories(BEFORE
    ${CMAKE_CURRENT_BINARY_DIR}/libkwineffects
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/libkwineffects
    ${CMAKE_CURRENT_SOURCE_DIR}/effects
    ${CMAKE_CURRENT_SOURCE_DIR}/tabbox
    )

add_subdirectory( libkwineffects )
add_subdirectory( killer )
if(KWIN_BUILD_KCMS)
    add_subdirectory( kcmkwin )
endif()

if( KWIN_BUILD_DECORATIONS )
    add_subdirectory( clients )
endif()
add_subdirectory( data )

add_subdirectory( effects )
add_subdirectory( scripts )
add_subdirectory( tabbox )
add_subdirectory(scripting)

add_definitions(-DKDE_DEFAULT_DEBUG_AREA=1212)

########### next target ###############

set(kwin_KDEINIT_SRCS
   workspace.cpp 
   dbusinterface.cpp
   abstract_client.cpp
   client.cpp 
   client_machine.cpp
   cursor.cpp
   tabgroup.cpp
   focuschain.cpp
   globalshortcuts.cpp
   input.cpp
   netinfo.cpp
   placement.cpp 
   atoms.cpp 
   utils.cpp 
   layers.cpp 
   main.cpp 
   options.cpp 
   outline.cpp
   events.cpp 
   killwindow.cpp 
   geometrytip.cpp 
   screens.cpp
   screens_xrandr.cpp
   shadow.cpp
   sm.cpp 
   group.cpp 
   manage.cpp 
   overlaywindow.cpp
   activation.cpp 
   useractions.cpp 
   geometry.cpp 
   rules.cpp
   composite.cpp
   toplevel.cpp
   unmanaged.cpp
   scene.cpp
   scene_xrender.cpp
   scene_opengl.cpp
   scene_qpainter.cpp
   glxbackend.cpp
   thumbnailitem.cpp
   lanczosfilter.cpp
   deleted.cpp
   effects.cpp
   effectloader.cpp
   compositingprefs.cpp
   virtualdesktops.cpp
   xcbutils.cpp
   x11eventfilter.cpp
   logind.cpp
    screenedge.cpp
    scripting/scripting.cpp
    scripting/workspace_wrapper.cpp
    scripting/meta.cpp
    scripting/scriptedeffect.cpp
    scripting/scriptingutils.cpp
    scripting/timer.cpp
    scripting/scripting_model.cpp
    scripting/dbuscall.cpp
    scripting/screenedgeitem.cpp
    decorations/decoratedclient.cpp
    decorations/decorationbridge.cpp
    decorations/decorationpalette.cpp
    decorations/settings.cpp
    decorations/decorationrenderer.cpp
   )

if(KWIN_BUILD_TABBOX)
    set(
        kwin_KDEINIT_SRCS ${kwin_KDEINIT_SRCS}
        tabbox/tabbox.cpp
        tabbox/clientmodel.cpp
        tabbox/desktopchain.cpp
        tabbox/desktopmodel.cpp
        tabbox/switcheritem.cpp
        tabbox/tabboxconfig.cpp
        tabbox/tabboxhandler.cpp
  )
endif()

if(KWIN_BUILD_ACTIVITIES)
    set(
        kwin_KDEINIT_SRCS ${kwin_KDEINIT_SRCS}
        activities.cpp
    )
endif()

if(KWIN_HAVE_EGL)
    set(kwin_KDEINIT_SRCS ${kwin_KDEINIT_SRCS} abstract_egl_backend.cpp eglonxbackend.cpp)
endif()

if(HAVE_WAYLAND)
    set(kwin_KDEINIT_SRCS
            ${kwin_KDEINIT_SRCS}
            abstract_backend.cpp
            virtual_terminal.cpp
            shell_client.cpp
            wayland_server.cpp
       )
    if(HAVE_WAYLAND_CURSOR)
        set(kwin_KDEINIT_SRCS
                ${kwin_KDEINIT_SRCS}
                wayland_cursor_theme.cpp
            )
    endif()
endif()

if(UDEV_FOUND)
    set(kwin_KDEINIT_SRCS
        ${kwin_KDEINIT_SRCS}
        udev.cpp
    )
endif()

if(HAVE_INPUT)
    set(kwin_KDEINIT_SRCS
        ${kwin_KDEINIT_SRCS}
        libinput/context.cpp
        libinput/connection.cpp
        libinput/events.cpp
    )
endif()

kconfig_add_kcfg_files(kwin_KDEINIT_SRCS settings.kcfgc)

qt5_add_dbus_adaptor( kwin_KDEINIT_SRCS org.kde.KWin.xml dbusinterface.h KWin::DBusInterface )
qt5_add_dbus_adaptor( kwin_KDEINIT_SRCS org.kde.kwin.Compositing.xml dbusinterface.h KWin::CompositorDBusInterface )
qt5_add_dbus_adaptor( kwin_KDEINIT_SRCS ${kwin_effects_dbus_xml} effects.h KWin::EffectsHandlerImpl )

qt5_add_dbus_interface( kwin_KDEINIT_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/org.freedesktop.ScreenSaver.xml screenlocker_interface)

qt5_add_resources( kwin_KDEINIT_SRCS resources.qrc )

ki18n_wrap_ui(kwin_KDEINIT_SRCS
    shortcutdialog.ui
)

########### target link libraries ###############

set(kwin_OWN_LIBS
    kwineffects
    kwinxrenderutils
    kwin4_effect_builtins
)

set(kwin_QT_LIBS
    Qt5::Concurrent
    Qt5::DBus
    Qt5::Quick
    Qt5::Script
    Qt5::X11Extras
)

set(kwin_KDE_LIBS
    KF5::ConfigCore
    KF5::CoreAddons
    KF5::ConfigWidgets
    KF5::Crash
    KF5::GlobalAccel
    KF5::I18n
    KF5::Notifications
    KF5::Service
    KF5::Plasma
    KF5::WindowSystem
    KDecoration2::KDecoration
    KDecoration2::KDecoration2Private
)

set(kwin_XLIB_LIBS
    ${X11_X11_LIB}
    ${X11_ICE_LIB}
    ${X11_SM_LIB}
)

set(kwin_XCB_LIBS
    XCB::XCB
    XCB::XFIXES
    XCB::DAMAGE
    XCB::COMPOSITE
    XCB::SHAPE
    XCB::SYNC
    XCB::RENDER
    XCB::RANDR
    XCB::KEYSYMS
    XCB::SHM
    XCB::GLX
)

set(kwin_WAYLAND_LIBS
    XKB::XKB
    KF5::WaylandClient
    KF5::WaylandServer
    ${CMAKE_THREAD_LIBS_INIT}
)

if(Wayland_Cursor_FOUND)
    set(kwin_WAYLAND_LIBS ${kwin_WAYLAND_LIBS} Wayland::Cursor)
endif()

if(KWIN_BUILD_ACTIVITIES)
    set(kwin_KDE_LIBS ${kwin_KDE_LIBS} KF5::Activities)
endif()

if(HAVE_XCB_CURSOR)
    set(kwin_XCB_LIBS ${kwin_XCB_LIBS} XCB::CURSOR)
endif()

set(kwinLibs
    ${kwin_OWN_LIBS}
    ${kwin_QT_LIBS}
    ${kwin_KDE_LIBS}
    ${kwin_XLIB_LIBS}
    ${kwin_XCB_LIBS}
)

if(HAVE_WAYLAND)
    set(kwinLibs ${kwinLibs} ${kwin_WAYLAND_LIBS})
endif()

if(UDEV_FOUND)
    set(kwinLibs ${kwinLibs} ${UDEV_LIBS})
endif()

if(HAVE_INPUT)
    set(kwinLibs ${kwinLibs} Libinput::Libinput)
endif()

add_library(kwin SHARED ${kwin_KDEINIT_SRCS})

set_target_properties(kwin PROPERTIES
   VERSION ${PROJECT_VERSION}
   SOVERSION ${PROJECT_VERSION_MAJOR}
   )

target_link_libraries(kwin ${kwinLibs})
generate_export_header(kwin EXPORT_FILE_NAME kwin_export.h)

if(KWIN_BUILD_OPENGL)
  target_link_libraries(kwin kwinglutils ${epoxy_LIBRARY})
  # -ldl used by OpenGL code
  find_library(DL_LIBRARY dl)
  if (DL_LIBRARY)
    target_link_libraries(kwin ${DL_LIBRARY})
  endif()
elseif(KWIN_BUILD_OPENGLES)
  target_link_libraries(kwin ${kwinLibs} kwinglesutils ${epoxy_LIBRARY})
  set_target_properties(kwin PROPERTIES COMPILE_FLAGS "-DKWIN_HAVE_OPENGLES")
endif()

kf5_add_kdeinit_executable(kwin_x11 main_x11.cpp)
target_link_libraries(kdeinit_kwin_x11 kwin)

install(TARGETS kwin             ${INSTALL_TARGETS_DEFAULT_ARGS} LIBRARY NAMELINK_SKIP )
install(TARGETS kdeinit_kwin_x11 ${INSTALL_TARGETS_DEFAULT_ARGS} )
install(TARGETS kwin_x11         ${INSTALL_TARGETS_DEFAULT_ARGS} )

if(HAVE_WAYLAND)
    include_directories(${Qt5Core_PRIVATE_INCLUDE_DIRS})
    kf5_add_kdeinit_executable(kwin_wayland main_wayland.cpp)
    target_link_libraries(kdeinit_kwin_wayland kwin)

    install(TARGETS kdeinit_kwin_wayland ${INSTALL_TARGETS_DEFAULT_ARGS} )
    install(TARGETS kwin_wayland         ${INSTALL_TARGETS_DEFAULT_ARGS} )

    add_subdirectory(backends)
endif()

########### install files ###############

install( FILES kwin.kcfg  DESTINATION  ${KCFG_INSTALL_DIR} RENAME ${KWIN_NAME}.kcfg )
install( FILES kwin.notifyrc  DESTINATION  ${KNOTIFYRC_INSTALL_DIR} RENAME ${KWIN_NAME}.notifyrc)
install(
    FILES
        org.kde.KWin.xml
        org.kde.kwin.Compositing.xml
        org.kde.kwin.Effects.xml
    DESTINATION
        ${DBUS_INTERFACES_INSTALL_DIR}
)
install( FILES ${CMAKE_CURRENT_BINARY_DIR}/kwin_export.h DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel)

# Install the KWin/Script service type
install( FILES scripting/kwinscript.desktop DESTINATION ${SERVICETYPES_INSTALL_DIR} )

ecm_install_icons(
    ICONS
        16-apps-kwin.png
        32-apps-kwin.png
        48-apps-kwin.png
        sc-apps-kwin.svgz
    DESTINATION
        ${ICON_INSTALL_DIR}
    THEME
        hicolor
)

add_subdirectory(qml)
add_subdirectory(autotests)
add_subdirectory(tests)

if (KF5DocTools_FOUND)
    add_subdirectory(doc)
endif()

feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)

include(ECMPackageConfigHelpers)
set(CMAKECONFIG_INSTALL_DIR "${CMAKECONFIG_INSTALL_PREFIX}/KWinDBusInterface")
ecm_configure_package_config_file(KWinDBusInterfaceConfig.cmake.in
    "${CMAKE_CURRENT_BINARY_DIR}/KWinDBusInterfaceConfig.cmake"
    INSTALL_DESTINATION ${CMAKECONFIG_INSTALL_DIR})
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/KWinDBusInterfaceConfig.cmake
    DESTINATION ${CMAKECONFIG_INSTALL_DIR})
