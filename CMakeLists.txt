cmake_minimum_required(VERSION 3.28)
project(Bookstore_2025 CXX)

set(CMAKE_C_STANDARD 11)

include_directories(.)
include_directories(include)

add_executable(Bookstore_2025
        include/Types.h
        include/Utils.h
        include/Utils.h
        include/CmdParser.h
        include/AccountManager.h
        include/BookManager.h
        include/Logger.h
        include/Persistence.h
        scr/Persistence.cpp
        scr/Utils.cpp
        scr/AccountManager.cpp
        scr/BookManager.cpp
        scr/CmdParser.cpp
        scr/Logger.cpp
        scr/Persistence.cpp
        scr/main.cpp
        include/Transaction.h
        scr/Transaction.cpp)
