cmake_minimum_required(VERSION 3.0)

project(webserver)

SET(CMAKE_BUILD_TYPE "Debug")  
SET(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g2 -ggdb")  
SET(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O3 -Wall") 



aux_source_directory(./source DIR_SR)

add_executable(webserver ${DIR_SR})