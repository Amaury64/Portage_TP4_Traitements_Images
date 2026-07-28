cmake_minimum_required(VERSION 3.20)
project(Portage_TP4_Traitements_Images LANGUAGES CXX)

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
# Obligatoire sous MSVC : sans cela, GoogleTest utilise un runtime statique
# incompatible avec le projet et l'edition de liens echoue.
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# OpenCV n'est PAS telecharge : find_package cherche une bibliotheque deja
# installee. En local c'est vcpkg qui la fournit (via CMakePresets), en CI
# c'est le paquet systeme libopencv-dev.
find_package(OpenCV REQUIRED)

# --- Bibliotheques du projet ----------------------------------------------
# pieces : calcul pur (calibration, classification). Aucune dependance image.
add_library(pieces src/pieces.cpp)
target_include_directories(pieces PUBLIC include)

# vision : traitement d'image. Seule cible liee a OpenCV.
add_library(vision src/vision.cpp)
target_include_directories(vision PUBLIC include)

target_link_libraries(vision PUBLIC ${OpenCV_LIBS} pieces)

# --- Executable -----------------------------------------------------------
add_executable(compter src/main.cpp)
target_link_libraries(compter PRIVATE vision)

# --- Tests ----------------------------------------------------------------
enable_testing()
include(GoogleTest)

add_executable(tests_pieces tests/test_pieces.cpp)
target_link_libraries(tests_pieces PRIVATE pieces GTest::gtest_main)
gtest_discover_tests(tests_pieces)

add_executable(tests_vision tests/test_vision.cpp)
target_link_libraries(tests_vision PRIVATE vision GTest::gtest_main)
gtest_discover_tests(tests_vision)

# --- Options de compilation -----------------------------------------------
if(MSVC)
    target_compile_options(pieces       PRIVATE /W4 /permissive-)
    target_compile_options(vision       PRIVATE /W4 /permissive-)
    target_compile_options(tests_pieces PRIVATE /W4 /permissive-)
    target_compile_options(tests_vision PRIVATE /W4 /permissive-)
    target_compile_options(compter      PRIVATE /W4 /permissive-)
else()
    target_compile_options(pieces       PRIVATE -Wall -Wextra -Wpedantic)
    target_compile_options(vision       PRIVATE -Wall -Wextra -Wpedantic)
    target_compile_options(tests_pieces PRIVATE -Wall -Wextra -Wpedantic)
    target_compile_options(tests_vision PRIVATE -Wall -Wextra -Wpedantic)
    target_compile_options(compter      PRIVATE -Wall -Wextra -Wpedantic)
endif()