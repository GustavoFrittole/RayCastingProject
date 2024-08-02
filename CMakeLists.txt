cmake_minimum_required(VERSION 3.16)
project(CMakeSFMLProject LANGUAGES CXX)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
option(BUILD_SHARED_LIBS "Build shared libraries" OFF)

#copy assets folder in root
set(SOURCE_DIR "${CMAKE_SOURCE_DIR}/assets")
set(DEST_DIR "${CMAKE_BINARY_DIR}")
file(COPY ${SOURCE_DIR} DESTINATION ${DEST_DIR})

include(FetchContent)
## search for sfml
find_package(SFML 2.6 COMPONENTS system window graphics QUIET)

if (NOT SFML_FOUND)
    FetchContent_Declare(
        SFML
        GIT_REPOSITORY https://github.com/SFML/SFML.git
        GIT_TAG 2.6.x)
    FetchContent_MakeAvailable(SFML)
endif()

## json parser
FetchContent_Declare(
    json 
    URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz)
FetchContent_MakeAvailable(json)

add_executable(main src/main.cpp)

include_directories(include)

add_subdirectory(libs)

target_link_libraries(gameCore PRIVATE utils mapGenerator dataManager)
target_link_libraries(gameGraphics PUBLIC gameCore sfml-graphics utils mapGenerator pathFinder)
target_link_libraries(dataManager PRIVATE nlohmann_json::nlohmann_json)
target_link_libraries(gameHandler PRIVATE gameGraphics)
target_link_libraries(main PRIVATE gameHandler)

target_compile_features(main PRIVATE cxx_std_17)
