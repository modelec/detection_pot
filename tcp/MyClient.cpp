#include "MyClient.h"

MyClient::~MyClient()
{
    this->stop();
}

MyClient::MyClient(Type::RobotPose* robotPose, const char* ip, const int port) : TCPClient(ip, port), robotPose(robotPose)
{

}

void MyClient::handleMessage(const std::string& message)
{
    std::vector<std::string> messageSplited = split(message, ";");

    if (messageSplited[0] == "aruco" || messageSplited[0] == "all")
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
                    res += std::to_string(pos.first.at<double>(1, 0)) + ",";
                    res += std::to_string(pos.first.at<double>(2, 0)) + ",";
                }
            }

            this->sendMessage(res.c_str());
        } else if (messageSplited[2] == "ping")
        {
            this->sendMessage("pong aruco");
        } else if (messageSplited[2] == "set robot_pos")
        {
            // cut the string with space and take the first, second, third and fourth element
            std::vector<std::string> tokens = split(message, " ");

            robotPose->position.x = std::stof(tokens[1]);
            robotPose->position.y = std::stof(tokens[2]);
            robotPose->position.z = std::stof(tokens[3]);
            robotPose->theta = std::stof(tokens[4]);
        }
    }
}

void MyClient::setArucoTags(const std::vector<std::pair<ArucoTag, std::pair<cv::Mat, cv::Mat>>>& arucoTags)
{
    this->arucoTags = arucoTags;
}
