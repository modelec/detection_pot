#pragma once

#include "utils.h"
#include "ArucoTag.h"

class ArucoDetector {
    std::vector<ArucoTag> arucoTags;

    Type::RobotPose robotPose;

    cv::Mat cameraMatrix;
    cv::Mat distCoeffs;

    cv::VideoCapture cap;

    cv::aruco::ArucoDetector detector;

    bool headless;

    cv::Mat transformationMatrix;

public:
    ArucoDetector(Type::RobotPose pose, const std::string& calibrationPath, int cameraId = 0, bool headless = false);

    ArucoDetector(float x, float y, float z, float theta, const std::string& calibrationPath, int cameraId = 0, bool headless = false);

    ~ArucoDetector();

    int detectArucoTags();

    void readCameraParameters(const std::string& path);

    void addArucoTag(const ArucoTag& tag);

    void flowerDetector(const ArucoTag& type, const cv::Mat& translationMatrix, const cv::Mat& rotationMatrix);

    void solarPanelDetector(ArucoTag type, cv::Mat translationMatrix, cv::Mat rotationMatrix);

};
