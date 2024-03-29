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
    std::cout << "Message From My Client" << std::endl;
    std::cout << message << std::endl;
    if (startWith(message, "request aruco"))
    {
        std::string res;

        for (auto& [tag, pos] : arucoTags)
        {
            res += std::to_string(tag.id) + " ";
            res += std::to_string(pos.first.at<float>(0, 0)) + " ";
            res += std::to_string(pos.first.at<float>(1, 0)) + " ";
            res += std::to_string(pos.first.at<float>(2, 0)) + " ";
        }

        this->sendMessage(res.c_str());
    } else if (startWith(message, "ping aruco") || startWith(message, "ping all"))
    {
        this->sendMessage("pong aruco");
    } else if (startWith(message, "robotPose"))
    {
        // cut the string with space and take the first, second, third and fourth element
        std::vector<std::string> tokens = split(message, " ");

        robotPose->position.x = std::stof(tokens[1]);
        robotPose->position.y = std::stof(tokens[2]);
        robotPose->position.z = std::stof(tokens[3]);
        robotPose->theta = std::stof(tokens[4]);
    }
}

void MyClient::setArucoTags(const std::vector<std::pair<ArucoTag, std::pair<cv::Mat, cv::Mat>>>& arucoTags)
{
    this->arucoTags = arucoTags;
}
