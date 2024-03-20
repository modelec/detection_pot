#include "utils.h"

double distanceBetweenRobotAndTag(const Type::RobotPose& robotPose, const cv::Mat& a)
{
    return sqrt(pow(robotPose.position.x + a.at<double>(0, 0), 2) + pow(robotPose.position.y + a.at<double>(1, 0), 2) + pow(robotPose.position.z + a.at<double>(2, 0), 2));
}
