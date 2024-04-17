#include "utils.h"

double distanceBetweenRobotAndTagOnXY(const cv::Mat& a)
{
    return sqrt(pow(a.at<double>(0, 0), 2) + pow(a.at<double>(2, 0), 2));
}