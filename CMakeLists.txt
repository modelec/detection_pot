cmake_minimum_required(VERSION 3.25)
project(detection_pot VERSION 1.0.1)

set(CMAKE_CXX_STANDARD 17)

find_package( OpenCV REQUIRED )
include_directories( ${OpenCV_INCLUDE_DIRS} )

set(COMMON_SOURCES
        utils/utils.h
        utils/utils.cpp
)

set(calibrationSources
        ${COMMON_SOURCES}
        calibration.cpp
)

add_executable(calibration ${calibrationSources})

target_link_libraries( calibration ${OpenCV_LIBS} )

# Project 2 sources and executable
set(arucoDetectionSources
        ${COMMON_SOURCES}
        arucoDetector.cpp
        aruco/ArucoTag.cpp
        aruco/ArucoTag.h
        aruco/ArucoDetector.cpp
        aruco/ArucoDetector.h
)

add_executable(arucoDetector ${arucoDetectionSources})

target_link_libraries( arucoDetector ${OpenCV_LIBS} )