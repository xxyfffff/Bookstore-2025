cmake_minimum_required(VERSION 3.28)
project(Bookstore_2025 CXX)

set(CMAKE_C_STANDARD 11)

include_directories(.)
include_directories(include)

add_executable(Bookstore_2025
        include/Types.h
        include/Utils.h
        scr/Utils.cpp
        include/Utils.h
        include/CmdParser.h
        include/AccountManager.h
        include/BookManager.h
        include/Logger.h
        scr/AccountManager.cpp
        scr/BookManager.cpp
        scr/CmdParser.cpp
        scr/Logger.cpp)
