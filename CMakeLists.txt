cmake_minimum_required(VERSION 3.5.1)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(ptrexample)

set(SOURCES ptrexample.cpp)

add_executable(${PROJECT_NAME} ${SOURCES})
