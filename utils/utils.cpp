#include "utils.h"

double distanceBetweenRobotAndTag(const cv::Mat& a)
{
    return sqrt(pow(a.at<double>(0, 0), 2) + pow(a.at<double>(1, 0), 2) + pow(a.at<double>(2, 0), 2));
}