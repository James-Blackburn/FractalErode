cmake_minimum_required(VERSION 3.28.1)
project(stb)

add_library(stb INTERFACE)
target_include_directories(stb INTERFACE ./)
