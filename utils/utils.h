#pragma once

#include <opencv2/opencv.hpp>

namespace Type
{
    struct Angle
    {
        float roll;
        float pitch;
        float yaw;
    };
}

enum Team
{
    BLUE,
    YELLOW
};

double distanceBetweenRobotAndTagOnXY(const cv::Mat& a);