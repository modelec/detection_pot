#include "MyClient.h"

MyClient::MyClient(const char* ip, const int port) : TCPClient(ip, port)
{

}

void MyClient::handleMessage(const std::string& message)
{
    std::vector<std::string> messageSplited = TCPSocket::split(message, ";");

    if (messageSplited[1] == "aruco" || messageSplited[1] == "all")
    {
        if (messageSplited[2] == "get aruco")
        {
            std::string res;

            res += "aruco;strat;get aruco;";

            if (arucoTags.empty())
            {
                res += "404";
            } else {
                for (auto& [tag, pos] : arucoTags)
                {
                    res += std::to_string(tag.id) + ",";
                    res += tag.name + ",";
                    res += std::to_string(pos.first.at<double>(0, 0)) + ",";
                    res += std::to_string(pos.first.at<double>(2, 0)) + ",";
                }
            }

            this->sendMessage(res.c_str());
        } else if (messageSplited[2] == "ping")
        {
            this->sendMessage("aruco;ihm;pong;1");
        }
    }
}

void MyClient::setArucoTags(const std::vector<std::pair<ArucoTag, std::pair<cv::Mat, cv::Mat>>>& arucoTags)
{
    this->arucoTags = arucoTags;
}
