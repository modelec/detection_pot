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

double distanceBetweenRobotAndTag(Type::RobotPose robotPose, const cv::Mat& a);

inline bool startWith(const std::string& str, const std::string& start)
{
    return str.rfind(start, 0) == 0;
}

inline bool endsWith(const std::string& str, const std::string& end)
{
    if (str.length() >= end.length())
    {
        return (0 == str.compare(str.length() - end.length(), end.length(), end));
    }
    return false;
}

inline bool contains(const std::string& str, const std::string& sub)
{
    return str.find(sub) != std::string::npos;
}

inline std::vector<std::string> split(const std::string& str, const std::string& delimiter)
{
    std::vector<std::string> tokens;
    size_t prev = 0, pos = 0;
    do
    {
        pos = str.find(delimiter, prev);
        if (pos == std::string::npos) pos = str.length();
        std::string token = str.substr(prev, pos - prev);
        if (!token.empty()) tokens.push_back(token);
        prev = pos + delimiter.length();
    } while (pos < str.length() && prev < str.length());
    return tokens;
}