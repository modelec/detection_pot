#pragma once

#include <TCPSocket/TCPClient.hpp>
#include <TCPSocket/TCPUtils.hpp>

#include "../utils/utils.h"
#include "../aruco/ArucoTag.h"

class MyClient : public TCPClient {
Type::RobotPose* robotPose;
std::vector<std::pair<ArucoTag, std::pair<cv::Mat, cv::Mat>>> arucoTags;

public:

    virtual ~MyClient();

    MyClient(Type::RobotPose* robotPose, const char* ip = "127.0.0.1", int port = 8080);

    void handleMessage(const std::string& message) override;

    void setArucoTags(const std::vector<std::pair<ArucoTag, std::pair<cv::Mat, cv::Mat>>>& arucoTags);
};
