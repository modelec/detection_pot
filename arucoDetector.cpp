#include <opencv2/opencv.hpp>

#include "utils/ArucoDetector.h"

int main(int argc, char *argv[])
{
    // Settup argument parser

    if (argc < 3) {
        std::cout << "Usage: " << argv[0] << " <video capture device> <path/to/calibration_results.yaml>" << std::endl;
        return 1;
    }

    bool headless = false;

    for (int i = 0; i < argc; i++)
    {
        if (std::string(argv[i]) == "--headless")
        {
            std::cout << "Running in headless mode." << std::endl;
            headless = true;
        }
    }

    const int cameraId = std::stoi(argv[1]);

    if (cameraId < 0)
    {
        std::cerr << "Error: Camera ID must be a positive integer." << std::endl;
        return -1;
    }

    const std::string calibrationPath = argv[2];

    // End argument parser

    const auto robotPose = Type::RobotPose{cv::Point3f(0, 0, 0), CV_PI/2};

    ArucoDetector detector(robotPose, calibrationPath, BLUE, cameraId, headless);

    while (true) {
        const auto res = detector.detectArucoTags();

        if (res.first == -2)
        {
            std::cerr << "Error: Could not capture frame." << std::endl;
            return -2;
        }

        if (res.first == 1)
        {
            break;
        }

        for (auto p : res.second)
        {
            if (p.first.type == FLOWER)
            {
                ArucoDetector::flowerDetector(p.first, p.second.first, p.second.first);
            } else if (p.first.type == SOLAR_PANEL)
            {
                ArucoDetector::solarPanelDetector(p.first, p.second.first, p.second.first, robotPose);
            }
        }
    }

    return 0;
}