cmake_minimum_required(VERSION 2.8)
project(RecvTestUbuntu)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

find_package(OpenCV REQUIRED)

set(SOURCE_FILES main.cpp ImagePacket.h)
add_executable(RecvTestUbuntu ${SOURCE_FILES})
target_link_libraries(RecvTestUbuntu ${OpenCV_LIBRARIES})
