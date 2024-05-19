#pragma once

#include <Modelec/TCPClient.h>
#include <Modelec/Utils.h>

#include "../utils/utils.h"
#include "../aruco/ArucoTag.h"

class MyClient : public TCPClient {

    std::vector<std::pair<ArucoTag, std::pair<cv::Mat, cv::Mat>>> arucoTags;

public:

    explicit MyClient(const char* ip = "127.0.0.1", int port = 8080);

    void handleMessage(const std::string& message) override;

    void setArucoTags(const std::vector<std::pair<ArucoTag, std::pair<cv::Mat, cv::Mat>>>& arucoTags);
};
