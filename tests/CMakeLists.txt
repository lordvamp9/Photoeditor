include(FetchContent)

FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

add_executable(photostudio_tests
    unit/test_blend_modes.cpp
    unit/test_document.cpp
    unit/test_filters.cpp
    unit/test_project_io.cpp
)

target_link_libraries(photostudio_tests PRIVATE
    photostudio_core
    GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(photostudio_tests)
