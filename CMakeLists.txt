cmake_minimum_required(VERSION 3.16)
project(CMakeSFMLProject LANGUAGES CXX)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
option(BUILD_SHARED_LIBS "Build shared libraries" OFF)
set(CMAKE_BUILD_TYPE "Release")

# copy assets folder in root (useful when working with some IDEs)
set(SOURCE_DIR "${CMAKE_SOURCE_DIR}/assets")
set(DEST_DIR "${CMAKE_BINARY_DIR}")
file(COPY ${SOURCE_DIR} DESTINATION ${DEST_DIR})

include(FetchContent)

# search for sfml
find_package(SFML 2.6 COMPONENTS system window graphics QUIET)

# fetch if not found
if (NOT SFML_FOUND)
    FetchContent_Declare(
        SFML
        GIT_REPOSITORY https://github.com/SFML/SFML.git
        GIT_TAG 2.6.x)
    FetchContent_MakeAvailable(SFML)
endif()

# fetch json parser
FetchContent_Declare(
    json 
    URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz)
FetchContent_MakeAvailable(json)

add_executable(demo src/demo.cpp)
add_library(gameDataStructures INTERFACE)

add_subdirectory(libs)

#-----------------------INCLUDES---------------------

target_include_directories(
    utils 
    PUBLIC ${CMAKE_SOURCE_DIR}/include/utils
)
target_include_directories(
    gameDataStructures
    INTERFACE   ${CMAKE_SOURCE_DIR}/include/engine  
                ${CMAKE_SOURCE_DIR}/include/utils
)
target_include_directories(
    mapGenerator 
    PUBLIC ${CMAKE_SOURCE_DIR}/include/utils)

target_include_directories(
    pathFinder 
    PUBLIC ${CMAKE_SOURCE_DIR}/include/utils
)
target_include_directories(
    gameInputs 
    PUBLIC ${CMAKE_SOURCE_DIR}/include/engine
)
target_include_directories(
    rendThreadPool
    PUBLIC ${CMAKE_SOURCE_DIR}/include/utils
)
target_include_directories(
    gameCore 
    PUBLIC  ${CMAKE_SOURCE_DIR}/include/engine 
            ${CMAKE_SOURCE_DIR}/include/utils
)
target_include_directories(
    gameGraphics 
    PUBLIC ${CMAKE_SOURCE_DIR}/include/engine
)
target_include_directories(
    dataManager 
    PUBLIC  ${CMAKE_SOURCE_DIR}/include/engine 
            ${CMAKE_SOURCE_DIR}/include/utils
)
target_include_directories(
    gameHandler 
    PUBLIC   ${CMAKE_SOURCE_DIR}/include
    PRIVATE  ${CMAKE_SOURCE_DIR}/include/engine
)

#---------------------LINKS------------------

target_link_libraries(
    gameDataStructures 
    INTERFACE utils 
)
target_link_libraries(
    gameCore 
    PRIVATE utils 
            mapGenerator
)
target_link_libraries(
    gameGraphics 
    PUBLIC  sfml-graphics 
            utils 
            pathFinder
            rendThreadPool
)
target_link_libraries(
    dataManager  
    PRIVATE nlohmann_json::nlohmann_json
    PUBLIC  gameDataStructures
)
target_link_libraries(
    gameHandler 
    PUBLIC  gameDataStructures
    PRIVATE gameGraphics 
            gameCore 
            gameInputs
            dataManager
            sfml-graphics
)
target_link_libraries(
    gameInputs 
    PUBLIC  sfml-graphics
            gameDataStructures
)
target_link_libraries(
    demo 
    PRIVATE gameHandler
)

target_compile_features(demo PRIVATE cxx_std_17)
