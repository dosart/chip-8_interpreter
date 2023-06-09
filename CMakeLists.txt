###################################################################################################
##
##      Information about the project
##
###################################################################################################

cmake_minimum_required(VERSION 3.16)

set(PROJECT_NAME chip8_interpreter)
project(${PROJECT_NAME} VERSION 1.0.0
        DESCRIPTION "Fifteen puzzle game"
        LANGUAGES CXX
        )

###################################################################################################
##
##      Compilation options
##
###################################################################################################

add_compile_options(
        -Werror

        -Wall
        -Wextra
        -Wpedantic

        -Wcast-align
        -Wcast-qual
        -Wconversion
        -Wctor-dtor-privacy
        -Wenum-compare
        -Wfloat-equal
        -Wnon-virtual-dtor
        -Wold-style-cast
        -Woverloaded-virtual
        -Wredundant-decls
        -Wsign-conversion
        -Wsign-promo
)

###################################################################################################
##
##      SDL2 link
##
###################################################################################################

find_package(SDL2 REQUIRED)

###################################################################################################
##
##      Source files
##
###################################################################################################

set(PROJECT_INCLUDE_DIR include)

add_executable(${PROJECT_NAME})
set_target_properties(
        ${PROJECT_NAME} PROPERTIES
        CXX_STANDARD 20
        CXX_STANDARD_REQUIRED ON
)

target_sources(
        ${PROJECT_NAME}
        PRIVATE
        src/main.cpp
        src/opcodes.cpp
        src/chip8.cpp
        src/viewer.cpp
)
target_include_directories(
        ${PROJECT_NAME}
        PRIVATE
        ${PROJECT_INCLUDE_DIR}
        ${SDL2_INCLUDE_DIRS}
)

target_link_libraries(${PROJECT_NAME} PRIVATE SDL2::SDL2)