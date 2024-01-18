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

    struct RobotPose
    {
        cv::Point3f position;
        float theta; // rotation around the y axis
    };
}

enum Team
{
    BLUE,
    YELLOW
};

double distance(cv::Mat a)
{

}
