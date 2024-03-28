#include "aruco/ArucoDetector.h"
#include "tcp/MyClient.h"

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

    bool headless = false;

    for (int i = 0; i < argc; i++)
    {
        if (std::string(argv[i]) == "--help")
        {
            std::cout << "Usage: " << argv[0] << "<path/to/calibration_results.yaml> <TCP port>" << std::endl;
            std::cout << "video capture device: The ID of the video capture device to use. Usually 0 for the built-in camera." << std::endl;
            std::cout << "path/to/calibration_results.yaml: The path to the calibration results file." << std::endl;
            std::cout << "to run the program in headless mode, add the --headless flag." << std::endl;
            return 0;
        }

        if (std::string(argv[i]) == "--headless")
        {
            std::cout << "Running in headless mode." << std::endl;
            headless = true;
        }
    }

    if (argc < 3) {
        std::cout << "Usage: " << argv[0] << "<path/to/calibration_results.yaml> <port>" << std::endl;
        return 1;
    }

    const std::string calibrationPath = argv[1];

    // End argument parser
    std::optional<std::thread> userInput;

    if (headless)
    {
        userInput = std::thread(userInputThread);
    }

    auto* robotPose = new Type::RobotPose{cv::Point3f(0, 0, 0), CV_PI/2};

    ArucoDetector detector(robotPose, calibrationPath, BLUE, headless);

    auto whiteFlower = ArucoTag(36, "White_flower", 20, FLOWER);
    //whiteFlower.setFlowerObjectRepresentation();
    auto purpleFlower = ArucoTag(13, "Purple_flower", 20, FLOWER);
    //purpleFlower.setFlowerObjectRepresentation();

    auto solarPanel = ArucoTag(47, "Solar_panel", 50, SOLAR_PANEL);

    int code;

    int port = std::stoi(argv[2]);

    MyClient client(robotPose, "127.0.0.1", port);

    client.start();

    client.sendMessage("aruco;strat;get robot_pose;0");

    while (true) {
        auto r = detector.detectArucoTags({whiteFlower, purpleFlower, solarPanel});

        code = r.first;

        client.setArucoTags(r.second);

        if (code == -2)
        {
            std::cerr << "Error: Could not capture frame." << std::endl;
            return -2;
        }

        if (code == 1)
        {
            stopRequested = true;
        }

        for (const auto& [tags, matrix] : r.second)
        {
            if (tags.type == FLOWER) {
                //ArucoDetector::flowerDetector(tags, matrix.first, matrix.first, robotPose);
            }
            else if (tags.type == SOLAR_PANEL)
            {
                //ArucoDetector::solarPanelDetector(tags, matrix.first, matrix.first, robotPose);
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