cmake_minimum_required(VERSION 3.0.0)
project(H264DLL)

add_library(H264DLL SHARED H264DLL.c)
target_link_libraries(H264DLL x264)
