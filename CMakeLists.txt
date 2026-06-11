cmake_minimum_required(VERSION 3.20)

project(PhotoStudio
    VERSION 1.0.0
    DESCRIPTION "PhotoStudio Premium - Professional photo editor by vamp9"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

option(PHOTOSTUDIO_BUILD_TESTS "Build unit tests" ON)
option(PHOTOSTUDIO_ENABLE_ASAN "Enable Address Sanitizer" OFF)

include(CompilerFlags)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Svg)
find_package(OpenCV REQUIRED COMPONENTS core imgproc imgcodecs photo)
find_package(Threads REQUIRED)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

if(PHOTOSTUDIO_ENABLE_ASAN)
    include(Sanitizers)
endif()

add_subdirectory(src)

if(PHOTOSTUDIO_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

message(STATUS "--------------------------------------------------")
message(STATUS "PhotoStudio ${PROJECT_VERSION} (vamp9)")
message(STATUS "C++ Standard : ${CMAKE_CXX_STANDARD}")
message(STATUS "Build type   : ${CMAKE_BUILD_TYPE}")
message(STATUS "Qt6          : ${Qt6_VERSION}")
message(STATUS "OpenCV       : ${OpenCV_VERSION}")
message(STATUS "--------------------------------------------------")
