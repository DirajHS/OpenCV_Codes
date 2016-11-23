cmake_minimum_required(VERSION 3.6)
project(bgfg)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(OpenCV_DIR "$OpenCV_INCLUDE_DIRS")
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/")

find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

set(SOURCE_FILES bgfg.cpp)
add_executable(bgfg ${SOURCE_FILES})


set(OpenCV_LIBS opencv_core opencv_imgproc opencv_highgui opencv_videoio opencv_imgcodecs opencv_video)
target_link_libraries(bgfg ${OpenCV_LIBS})