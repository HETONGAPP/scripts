cmake_minimum_required(VERSION 3.5)
project(ivero_backend_server_test)

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
        ${OpenCV_INCLUDE_DIRS}
        ${EIGEN3_INCLUDE_DIR}
)



set(SOURCES
    sender.cpp
)
set(SOURCES1
  reciever.cpp
)

set(SOURCES2
  reciever.cpp
)

set(SOURCES3
  calibration.cpp

)

set(SOURCES4
  pcl_sender.cpp

)


add_executable(${PROJECT_NAME} ${SOURCES})

target_include_directories(${PROJECT_NAME}
  PUBLIC ${ZeroMQ_INCLUDE_DIR}
)

target_link_libraries(${PROJECT_NAME}
  ${OpenCV_LIBS}
  ${realsense2_LIBRARY}
  realsense2::realsense2
  ${X11_LIBRARIES}
  dl
  ${ZeroMQ_LIBRARY}
  ${PCL_LIBRARIES}
  ${EIGEN3_LIBS}

)

add_executable(receiver ${SOURCES1})

target_include_directories(receiver
  PUBLIC ${ZeroMQ_INCLUDE_DIR}
)

target_link_libraries(receiver
  ${OpenCV_LIBS}
  ${realsense2_LIBRARY}
  realsense2::realsense2
  ${X11_LIBRARIES}
  dl
  ${ZeroMQ_LIBRARY}
  ${PCL_LIBRARIES}
  ${EIGEN3_LIBS}

)


add_executable(receiver2 ${SOURCES2})

target_include_directories(receiver2
  PUBLIC ${ZeroMQ_INCLUDE_DIR}
)

target_link_libraries(receiver2
  ${OpenCV_LIBS}
  ${realsense2_LIBRARY}
  realsense2::realsense2
  ${X11_LIBRARIES}
  dl
  ${ZeroMQ_LIBRARY}
  ${PCL_LIBRARIES}
  ${EIGEN3_LIBS}

)


add_executable(calibration ${SOURCES3})

target_include_directories(calibration
  PUBLIC ${ZeroMQ_INCLUDE_DIR}
)

target_link_libraries(calibration
  ${OpenCV_LIBS}
  ${realsense2_LIBRARY}
  realsense2::realsense2
  ${X11_LIBRARIES}
  dl
  ${ZeroMQ_LIBRARY}
  ${PCL_LIBRARIES}
  ${EIGEN3_LIBS}

)


add_executable(pcl_sender ${SOURCES4})

target_include_directories(pcl_sender
  PUBLIC ${ZeroMQ_INCLUDE_DIR}
)

target_link_libraries(pcl_sender
  ${OpenCV_LIBS}
  ${realsense2_LIBRARY}
  realsense2::realsense2
  ${X11_LIBRARIES}
  dl
  nlohmann_json::nlohmann_json
  ${ZeroMQ_LIBRARY}
  ${PCL_LIBRARIES}
  ${EIGEN3_LIBS}

)




set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_OPTIONS "-std=c++17")

