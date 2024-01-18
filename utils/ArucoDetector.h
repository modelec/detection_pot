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

    Team team;

public:
    // TODO triée les tag dans la liste en fonction de la distance

    ArucoDetector(Type::RobotPose pose, const std::string& calibrationPath, Team team, int cameraId = 0, bool headless = false);

    ArucoDetector(float x, float y, float z, float theta, const std::string& calibrationPath, Team team, int cameraId = 0, bool headless = false);

    ~ArucoDetector();

    std::pair<int, std::vector<std::pair<cv::Mat, cv::Mat>>> detectArucoTags();

    void readCameraParameters(const std::string& path);

    void addArucoTag(const ArucoTag& tag);

    void flowerDetector(const ArucoTag& type, const cv::Mat& translationMatrix, const cv::Mat& rotationMatrix);

    void solarPanelDetector(const ArucoTag& type, cv::Mat translationMatrix, const cv::Mat& rotationMatrix);

};
