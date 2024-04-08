#pragma once

#include <iostream>
#include <opencv2/opencv.hpp>
#include <opencv2/aruco.hpp>
#include <utility>

enum ArucoTagType
{
    FLOWER,
    SOLAR_PANEL
};

class ArucoTag {
public:
    int id;
    std::string name;
    float length;
    cv::Mat objectRepresenation;
    ArucoTagType type;

    ArucoTag(const int id, std::string name, const float length, const ArucoTagType type) : id(id), name(std::move(name)), length(length), type(type)
    {
        this->objectRepresenation = cv::Mat(4, 1, CV_32FC3);
        this->objectRepresenation.ptr<cv::Vec3f>(0)[0] = cv::Vec3f(-length/2.f, length/2.f, 0);
        this->objectRepresenation.ptr<cv::Vec3f>(0)[1] = cv::Vec3f(length/2.f, length/2.f, 0);
        this->objectRepresenation.ptr<cv::Vec3f>(0)[2] = cv::Vec3f(length/2.f, -length/2.f, 0);
        this->objectRepresenation.ptr<cv::Vec3f>(0)[3] = cv::Vec3f(-length/2.f, -length/2.f, 0);
    }

    void setFlowerObjectRepresentation()
    {
        // 18.96 w
        // 19.3 h
        this->objectRepresenation = cv::Mat(4, 1, CV_32FC3);
        this->objectRepresenation.ptr<cv::Vec3f>(0)[0] = cv::Vec3f(-19.f/2.f, 19.f/2.f, 0);
        this->objectRepresenation.ptr<cv::Vec3f>(0)[1] = cv::Vec3f(19.f/2.f, 19.f/2.f, 0);
        this->objectRepresenation.ptr<cv::Vec3f>(0)[2] = cv::Vec3f(19.f/2.f, -19.f/2.f, 0);
        this->objectRepresenation.ptr<cv::Vec3f>(0)[3] = cv::Vec3f(-19.f/2.f, -19.f/2.f, 0);
    }
};
