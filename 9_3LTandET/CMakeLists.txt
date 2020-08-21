cmake_minimum_required(VERSION 3.0)
SET(CMAKE_BUILD_TYPE "Debug")  
SET(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g2 -ggdb")  
SET(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O3 -Wall") 



aux_source_directory(./server SERVER_SOUR)
if(SERVER_SOUR)
    message(STATUS "find server source")
    project(server)
    add_executable(server ${SERVER_SOUR})
endif()



aux_source_directory(./client CLIENT_SOUR)
if(CLIENT_SOUR)
    message(STATUS "find client source")
    project(client)
    add_executable(client ${CLIENT_SOUR})
endif()

# SET(BUILD_PWD "${PROJECT_SOURCE_DIR}/build1")
# message(STATUS ${BUILD_PWD})

# IF(EXISTS ${BUILD_PWD})
#     message(STATUS "build exists")
# ELSE()
#     message(STATUS "build not exists")
# ENDIF()
# make_directory()
