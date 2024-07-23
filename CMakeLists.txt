cmake_minimum_required(VERSION 3.16)
project(CMakeSFMLProject LANGUAGES CXX)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
option(BUILD_SHARED_LIBS "Build shared libraries" OFF)

include(FetchContent)

FetchContent_Declare(SFML
    GIT_REPOSITORY https://github.com/SFML/SFML.git
    GIT_TAG 2.6.x)
FetchContent_MakeAvailable(SFML)

add_executable(main src/main.cpp)

include_directories(include)

add_subdirectory(libs)

target_link_libraries(gameCore PRIVATE utils)
target_link_libraries(gameGraphics PUBLIC gameCore sfml-graphics utils)
target_link_libraries(main PUBLIC gameGraphics)

target_compile_features(main PRIVATE cxx_std_17)
