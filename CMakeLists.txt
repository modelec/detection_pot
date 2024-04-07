cmake_minimum_required(VERSION 3.25)
project(detection_pot VERSION 1.0.1)

set(CMAKE_CXX_STANDARD 17)

find_package( OpenCV REQUIRED )
find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBCAMERA REQUIRED libcamera)
pkg_check_modules(TCPSocket REQUIRED TCPSocket)

include_directories(${LIBCAMERA_INCLUDE_DIRS} ${OPENCV_INCLUDE_DIRS})


set(COMMON_SOURCES
        utils/utils.h
        utils/utils.cpp
)

add_executable(calibration
        ${COMMON_SOURCES}
        calibration.cpp
)

add_executable(calibrationCharuco
        ${COMMON_SOURCES}
        calibrationCharuco.cpp
)

target_link_libraries( calibration -llccv ${OpenCV_LIBS} )

target_link_libraries( calibrationCharuco -llccv ${OpenCV_LIBS} )

# Project 2 sources and executable
set(arucoDetectionSources
        ${COMMON_SOURCES}
        arucoDetector.cpp
        aruco/ArucoTag.cpp
        aruco/ArucoTag.h
        aruco/ArucoDetector.cpp
        aruco/ArucoDetector.h
        tcp/MyClient.cpp
        tcp/MyClient.h
)

add_executable(arucoDetector ${arucoDetectionSources})

target_link_libraries( arucoDetector -llccv ${OpenCV_LIBS} )

target_link_libraries( arucoDetector TCPSocket )

set(photo
        ${COMMON_SOURCES}
        photo.cpp
)

add_executable(photo ${photo})

target_link_libraries( photo -llccv ${OpenCV_LIBS} )