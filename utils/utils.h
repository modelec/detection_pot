#pragma once

#include <iostream>
#include <limits>
#include <vector>
#include <opencv2/opencv.hpp>

namespace utils {
    void waitForUserInput()
    {
        std::cout << "Press Enter to continue...";
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    }
}


namespace Type
{
    struct Angle
    {
        float roll;
        float pitch;
        float yaw;
    };

    struct RobotPose
    {
        cv::Point3f position;
        Angle angle;
    };

    struct ArucoTag
    {
        int id;
        std::string name;
        float length;
    };
}

namespace ArucoTagData
{
    inline Type::ArucoTag whiteFlower = { 36, "whiteFlower", 0.02f };
    inline Type::ArucoTag purpleFlower = { 13, "purpleFlower", 0.02f };
    inline Type::ArucoTag solarPanel = { 47, "solarPanel", 0.1f };
}

namespace draw
{
    // Function to draw a line from the center of an ArUco marker to a point with depth
    void drawCenterPoints(cv::Mat& frame, const std::vector<std::vector<cv::Point2f>>& markerCorners, double depth) {
        for (const auto& corners : markerCorners) {
            cv::Point2f center(0, 0);
            for (const auto& corner : corners) {
                center += corner;
            }
            center *= (1.0 / corners.size());  // Average to find the center

            // Draw a line from the center of the marker to the point with depth
            cv::line(frame, center, cv::Point(center.x, center.y + depth), cv::Scalar(0, 255, 0), 2);  // Draw a green line
        }
    }
}