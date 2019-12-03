cmake_minimum_required(VERSION 3.13)
project(ServerClientAssignment)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS -pthread)

add_executable(ServerClientAssignment Server/WebServer.cpp Server/WebServer.h Server/RequestHandler.cpp Server/RequestHandler.h Client/main.cpp Server/main.cpp)