cmake_minimum_required(VERSION 3.5)
project(ivero_publisher)

IF(NOT CMAKE_BUILD_TYPE)
  SET(CMAKE_BUILD_TYPE Release)
ENDIF()

if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 17)
endif()

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}  -Wall   -O3")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall   -O3")
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -march=native")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -march=native")
# Add the -pthread flag to CMAKE_CXX_FLAGS
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
set(CMAKE_BUILD_TYPE "Debug")
#set(CMAKE_BUILD_TYPE "Release")


#############
## Prepare ##
#############

find_package(OpenCV REQUIRED)
find_package(X11 REQUIRED)
find_package(realsense2 REQUIRED)
find_package(PCL REQUIRED COMPONENTS common io filters)
find_package(Eigen3 3.3.7 REQUIRED) # this version may not fit  each devices, please switch the right wevsion 
find_package(nlohmann_json 3.2.0  REQUIRED)

pkg_check_modules(PC_ZeroMQ QUIET zmq)

## use the hint from above to find where 'zmq.hpp' is located
find_path(ZeroMQ_INCLUDE_DIR
        NAMES zmq.hpp
        PATHS ${PC_ZeroMQ_INCLUDE_DIRS}
        )

## use the hint from above to find the location of libzmq
find_library(ZeroMQ_LIBRARY
        NAMES zmq
        PATHS ${PC_ZeroMQ_LIBRARY_DIRS}
        )

###########
## Build ##
###########

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include/
    ${OpenCV_INCLUDE_DIRS}
    ${EIGEN3_INCLUDE_DIR}
)

add_executable(${PROJECT_NAME} src/ivero_publisher.cpp)

target_include_directories(${PROJECT_NAME}
  PUBLIC ${ZeroMQ_INCLUDE_DIR}
)

target_link_libraries(${PROJECT_NAME}
  dl  
  realsense2::realsense2
  nlohmann_json::nlohmann_json
  ${OpenCV_LIBS}
  ${realsense2_LIBRARY}
  ${X11_LIBRARIES}
  ${ZeroMQ_LIBRARY}
  ${PCL_LIBRARIES}
  ${EIGEN3_LIBS}
)

set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_OPTIONS "-std=c++17")

