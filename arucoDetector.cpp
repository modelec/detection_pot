#include "aruco/ArucoDetector.h"

#include <iostream>
#include <thread>
#include <atomic>
#include <optional>

std::atomic<bool> stopRequested(false);

void userInputThread() {
    // Wait for the user to press Enter
    std::cout << "Press Enter to stop the program..." << std::endl;
    std::cin.ignore();
    stopRequested = true;
}

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
    std::optional<std::thread> userInput;

    if (headless)
    {
        userInput = std::thread(userInputThread);
    }

    const auto robotPose = Type::RobotPose{cv::Point3f(0, 0, 0), CV_PI/2};

    ArucoDetector detector(robotPose, calibrationPath, BLUE, cameraId, headless);

    auto whiteFlower = ArucoTag(36, "White flower", 20, FLOWER);
    whiteFlower.setFlowerObjectRepresentation();
    auto purpleFlower = ArucoTag(13, "Purple flower", 20, FLOWER);
    purpleFlower.setFlowerObjectRepresentation();

    auto solarPanel = ArucoTag(47, "Solar panel", 50, SOLAR_PANEL);

    while (true) {
        const auto [code, res] = detector.detectArucoTags({whiteFlower, purpleFlower, solarPanel});

        if (code == -2)
        {
            std::cerr << "Error: Could not capture frame." << std::endl;
            return -2;
        }

        if (code == 1)
        {
            stopRequested = true;
        }

        for (auto [tags, matrix] : res)
        {
            if (tags.type == FLOWER)
            {
                ArucoDetector::flowerDetector(tags, matrix.first, matrix.first);
            } else if (tags.type == SOLAR_PANEL)
            {
                ArucoDetector::solarPanelDetector(tags, matrix.first, matrix.first, robotPose);
            }
        }

        if (stopRequested)
        {
            break;
        }
    }

    // Wait for the user input thread to finish
    if (userInput.has_value())
    {
        userInput.value().join();
    }

    return 0;
}