cmake_minimum_required(VERSION 3.25)
project(detection_pot VERSION 1.0.1)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package( OpenCV REQUIRED )
include_directories( ${OpenCV_INCLUDE_DIRS} )

find_package(PkgConfig REQUIRED)
pkg_check_modules(TCPSocket REQUIRED TCPSocket)

find_package(Qt6 COMPONENTS
        Core
        Network
        REQUIRED)

set(COMMON_SOURCES
        utils/utils.h
        utils/utils.cpp
)

set(calibrationSources
        ${COMMON_SOURCES}
        calibration.cpp
)

set(calibrationCharucoSources
        ${COMMON_SOURCES}
        calibrationCharuco.cpp
)

add_executable(calibration ${calibrationSources})
add_executable(calibrationCharuco ${calibrationCharucoSources})

target_link_libraries( calibration ${OpenCV_LIBS} )
target_link_libraries( calibrationCharuco ${OpenCV_LIBS} )

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

target_link_libraries( arucoDetector ${OpenCV_LIBS} )

target_link_libraries( arucoDetector Qt6::Core Qt6::Network )

target_link_libraries( arucoDetector TCPSocket )

set(photo
        ${COMMON_SOURCES}
        photo.cpp
)

add_executable(photo ${photo})

target_link_libraries( photo ${OpenCV_LIBS} )