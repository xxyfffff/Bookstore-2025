cmake_minimum_required(VERSION 3.28)
project(code CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR})

include_directories(.)
include_directories(include)

add_executable(code
        include/Types.h
        include/CmdParser.h
        include/AccountManager.h
        include/BookManager.h
        include/Persistence.h
        src/Persistence.cpp
        src/AccountManager.cpp
        src/BookManager.cpp
        src/CmdParser.cpp
        src/Persistence.cpp
        src/main.cpp
        include/Transaction.h
        src/Transaction.cpp)