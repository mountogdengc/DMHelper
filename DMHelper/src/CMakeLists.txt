cmake_minimum_required(VERSION 3.16)

project(DMHelper LANGUAGES CXX)

# -------------------------
# C++ standard
# -------------------------
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# -------------------------
# Qt setup (Qt5 or Qt6)
# -------------------------
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS
    Core Gui Widgets Xml Multimedia MultimediaWidgets OpenGL Network OpenGLWidgets UiTools
)

find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS
    Core Gui Widgets Xml Multimedia MultimediaWidgets OpenGL Network OpenGLWidgets UiTools
)

# -------------------------
# Enable Qt automoc/uic/rcc
# -------------------------
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# -------------------------
# Source files
# -------------------------
file(GLOB_RECURSE DMHELPER_SOURCES
    *.cpp
    ../../DMHelperShared/src/*.cpp
)

file(GLOB_RECURSE DMHELPER_HEADERS
    *.h
    ../../DMHelperShared/inc/*.h
    ../../DMHelperShared/src/*_private.h
)

file(GLOB_RECURSE DMHELPER_UI
    *.ui
    resources/ui/*.ui
)

file(GLOB_RECURSE DMHELPER_RESOURCES
    *.qrc
)

# -------------------------
# Application target
# -------------------------
add_executable(DMHelper
    ${DMHELPER_SOURCES}
    ${DMHELPER_HEADERS}
    ${DMHELPER_UI}
    ${DMHELPER_RESOURCES}
)

# -------------------------
# Include paths
# -------------------------
target_include_directories(DMHelper PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../../DMHelperShared/inc
    ${CMAKE_CURRENT_SOURCE_DIR}/../../DMHelperShared/src
)

# -------------------------
# Qt link libraries
# -------------------------
target_link_libraries(DMHelper PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Xml
    Qt${QT_VERSION_MAJOR}::Multimedia
    Qt${QT_VERSION_MAJOR}::MultimediaWidgets
    Qt${QT_VERSION_MAJOR}::OpenGL
    Qt${QT_VERSION_MAJOR}::OpenGLWidgets
    Qt${QT_VERSION_MAJOR}::Network
    Qt${QT_VERSION_MAJOR}::UiTools
)

# -------------------------
# Platform-specific config
# -------------------------

if (WIN32)
    target_compile_definitions(DMHelper PRIVATE WIN32)

    # Debug vs Release flags
    target_compile_options(DMHelper PRIVATE
        $<$<CONFIG:Debug>:/FS>
        $<$<CONFIG:Release>:/O2>
    )

    # VLC (Windows)
    if (CMAKE_SIZEOF_VOID_P EQUAL 4)
        message(STATUS "Using 32-bit VLC")
        target_include_directories(DMHelper PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/vlc32)
        target_link_directories(DMHelper PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/vlc32)
    else()
        message(STATUS "Using 64-bit VLC")
        target_include_directories(DMHelper PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/vlc64)
        target_link_directories(DMHelper PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/vlc64)
    endif()

    target_link_libraries(DMHelper PRIVATE libvlc)

    # Windows icon
    set_target_properties(DMHelper PROPERTIES
        WIN32_EXECUTABLE TRUE
    )

elseif (APPLE)
    message(STATUS "Configuring macOS build")

    target_compile_options(DMHelper PRIVATE
        $<$<CONFIG:Debug>:-g>
        $<$<CONFIG:Release>:-O2>
    )

    target_compile_definitions(DMHelper PRIVATE NDEBUG)

    # VLC (macOS)
    target_include_directories(DMHelper PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/vlcMac)
    target_link_directories(DMHelper PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/vlcMac)
    target_link_libraries(DMHelper PRIVATE vlc)

    # App bundle
    set_target_properties(DMHelper PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_ICON_FILE DMHelper.icns
    )

    set_source_files_properties(
        data/macimg/DMHelper.icns
        PROPERTIES MACOSX_PACKAGE_LOCATION Resources
    )

endif()

# -------------------------
# Definitions
# -------------------------
target_compile_definitions(DMHelper PRIVATE
    $<$<CONFIG:Release>:NDEBUG>
)

# -------------------------
# Warnings (optional but recommended)
# -------------------------
if (MSVC)
    target_compile_options(DMHelper PRIVATE /W3)
else()
    target_compile_options(DMHelper PRIVATE -Wall -Wextra)
endif()
