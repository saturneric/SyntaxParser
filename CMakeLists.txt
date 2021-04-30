cmake_minimum_required(VERSION 3.17)
project(syntaxParser)

set(CMAKE_CXX_STANDARD 14)

set(CMAKE_CXX_FLAGS_RELEASE "/Ox")

include_directories(./include)

aux_source_directory(src SRC_FILES)

add_executable(LR1Generator LR1Generator.cpp ${SRC_FILES})

add_executable(syntaxParser LR1Generator.cpp ${SRC_FILES})