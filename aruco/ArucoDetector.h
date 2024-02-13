#pragma once

#include "../utils/utils.h"
#include "ArucoTag.h"
#include <lccv.hpp>

class ArucoDetector {
    std::vector<ArucoTag> arucoTags;

    Type::RobotPose robotPose;

    cv::Mat cameraMatrix;
    cv::Mat distCoeffs;

    // cv::VideoCapture cap;
    lccv::PiCamera* cam;

    // 4.6
    cv::Ptr<cv::aruco::Dictionary> dictionary;

    // 4.8
    // cv::aruco::Dictionary dictionary;
    // cv::aruco::DetectorParameters parameters;
    // cv::aruco::ArucoDetector detector;

    bool headless;

    cv::Mat transformationMatrix;

    Team team;

    bool started = false;

public:
    ArucoDetector(const Type::RobotPose& pose, const std::string& calibrationPath, Team team, int cameraId = 0, bool headless = false);

    ArucoDetector(float x, float y, float z, float theta, const std::string& calibrationPath, Team team, int cameraId = 0, bool headless = false);

    ~ArucoDetector();

    std::pair<int, std::vector<std::pair<ArucoTag, std::pair<cv::Mat, cv::Mat>>>> detectArucoTags(std::vector<ArucoTag> tags = {});

    void readCameraParameters(const std::string& path);

    void addArucoTag(const ArucoTag& tag);

    static void flowerDetector(const ArucoTag& type, const cv::Mat& translationMatrix, const cv::Mat& rotationMatrix, const Type::RobotPose& robotPose);

    static void solarPanelDetector(const ArucoTag& type, cv::Mat translationMatrix, const cv::Mat& rotationMatrix, const Type::RobotPose& robotPose);

};
