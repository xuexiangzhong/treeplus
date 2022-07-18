cmake_minimum_required(VERSION 3.15)
project(treedb C)

set(CMAKE_C_STANDARD 99)

add_executable(breetest test/breetest.c src/BTreeFile.h src/BTreeFile.c)