cmake_minimum_required(VERSION 3.7 FATAL_ERROR)
project(cpp14 VERSION 0.0.0)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF) # Ensure std=C++11 not gnu++11

if (MSVC)
    # Enable all warnings
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Wall")

    # Disable C++ exceptions
    add_definitions (-D_HAS_EXCEPTIONS=0)
    string (REGEX REPLACE "/EHsc *" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /EHs-c-")
else ()
    # Enable all warnings
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -pedantic -Wextra")

    # Disable warnings about unknown pragmas (MSVC / GCC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unknown-pragmas")

    # Disable C++ exceptions
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-exceptions")
endif ()

include_directories(lib)
set(LIBRARY_FILES lib/linenoise/linenoise.hpp)

include_directories(src)
set(SOURCE_FILES src/main.cpp src/util/readline.cpp src/util/readline.h)

add_executable(step0_repl ${SOURCE_FILES} ${LIBRARY_FILES} src/step0_repl.cpp)
