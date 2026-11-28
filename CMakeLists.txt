cmake_minimum_required(VERSION 3.23)
# Single source of truth for the library version; release tags are v${PROJECT_VERSION}.
project(qcwdocking
    VERSION 2.0.0
    LANGUAGES CXX
)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    message(STATUS "Setting build type to 'Debug' as none was specified.")
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build." FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_definitions(QCWDOCK_DEBUG)
endif()

option(QCWDOCK_TREAT_WARNINGS_AS_ERRORS "Treat warnings as errors" ON)
if(QCWDOCK_TREAT_WARNINGS_AS_ERRORS)
    add_compile_options(
        $<$<COMPILE_LANGUAGE:CXX>:-Werror=uninitialized>
        $<$<COMPILE_LANGUAGE:CXX>:-Werror=return-type>
    )
endif()

# As a subproject of a host (QCW) only the library is built; the examples and tests
# are meant for a standalone checkout.
if(PROJECT_IS_TOP_LEVEL)
    set(QCWDOCK_STANDALONE_DEFAULT ON)
else()
    set(QCWDOCK_STANDALONE_DEFAULT OFF)
endif()
option(QCWDOCK_BUILD_TESTS "Build the test suite" ${QCWDOCK_STANDALONE_DEFAULT})
option(QCWDOCK_BUILD_EXAMPLES "Build the example programs" ${QCWDOCK_STANDALONE_DEFAULT})

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

find_package(QT NAMES Qt6 REQUIRED COMPONENTS Core)
find_package(Qt${QT_VERSION_MAJOR} 6.9 REQUIRED COMPONENTS
    Widgets
)

include(GNUInstallDirs)

add_subdirectory(src)

if(QCWDOCK_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

if(QCWDOCK_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

install(FILES LICENSE DESTINATION ${CMAKE_INSTALL_DATADIR}/licenses/qcw-docking)
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/translations)
    install(DIRECTORY translations DESTINATION ${CMAKE_INSTALL_DATADIR}/qcw)
endif()
