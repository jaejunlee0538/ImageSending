cmake_minimum_required(VERSION 2.8)
project(RecvTestUbuntu)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

find_package(OpenCV REQUIRED)
find_package(Boost COMPONENTS system REQUIRED)

include_directories(${Boost_INCLUDE_DIRS} ${OpenCV_INCLUDE_DIRS})
set(SOURCE_FILES main.cpp
        ImagePacket.h
        PreciseClock.h
        PreciseClock.cpp
        CVWindow.h
        CVWindow.cpp
        SerializeHelper.h
        )
add_executable(RecvTestUbuntu ${SOURCE_FILES})
target_link_libraries(RecvTestUbuntu ${OpenCV_LIBRARIES} ${Boost_LIBRARIES} pthread)
