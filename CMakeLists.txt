cmake_minimum_required(VERSION 3.20)
project(Portage_TP_Traitements_Images LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# --- Dependances externes -------------------------------------------------
include(FetchContent)

FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG        v1.15.2
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# --- Bibliotheque du projet -----------------------------------------------
add_library(pieces src/pieces.cpp)
target_include_directories(pieces PUBLIC include)

# --- Tests ----------------------------------------------------------------
enable_testing()

add_executable(tests tests/test_pieces.cpp)
target_link_libraries(tests PRIVATE pieces GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(tests)

# --- Options de compilation -----------------------------------------------
if(MSVC)
    target_compile_options(pieces PRIVATE /W4 /permissive-)
    target_compile_options(tests  PRIVATE /W4 /permissive-)
else()
    target_compile_options(pieces PRIVATE -Wall -Wextra -Wpedantic)
    target_compile_options(tests  PRIVATE -Wall -Wextra -Wpedantic)
endif()