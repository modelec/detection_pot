#include "aruco/ArucoDetector.h"
#include "tcp/MyClient.h"

#include <iostream>
#include <thread>
#include <atomic>
#include <optional>

int main(int argc, char *argv[])
{
    // Settup argument parser

    bool headless = false;

    bool stopRequested = false;

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

    ArucoDetector detector(calibrationPath, BLUE, headless);

    auto whiteFlower = ArucoTag(36, "White_flower", 19.6, FLOWER);
    // whiteFlower.setFlowerObjectRepresentation();log
    auto purpleFlower = ArucoTag(13, "Purple_flower", 19.6, FLOWER);
    // purpleFlower.setFlowerObjectRepresentation();

    auto solarPanel = ArucoTag(47, "Solar_panel", 36.8, SOLAR_PANEL);

    int code;

    int port = std::stoi(argv[2]);

    MyClient client("127.0.0.1", port);

    client.start();

    client.sendMessage("aruco;strat;ready;1");

    sleep(1);

    client.sendMessage("aruco;strat;get robot_pose;0");

    auto lastArucoFind = std::chrono::high_resolution_clock::now();

    while (true) {
        auto r = detector.detectArucoTags({whiteFlower, purpleFlower, solarPanel});

        code = r.first;

        if (!r.second.empty()) {
            client.setArucoTags(r.second);
        }

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
                std::cout << tags.id << " " << tags.name << " " << matrix.first.at<double>(2, 0) << " " << matrix.first.at<double>(0, 0) << std::endl;
                //ArucoDetector::flowerDetector(tags, matrix.first, matrix.first, robotPose);
                auto now = std::chrono::high_resolution_clock::now();
                std::cout << "Time since last find: " << std::chrono::duration_cast<std::chrono::milliseconds>(now - lastArucoFind).count() << "ms" << std::endl;

                lastArucoFind = now;
            }
            else if (tags.type == SOLAR_PANEL)
            {
                // std::cout << tags.id << " " << tags.name << " " << matrix.first.at<double>(2, 0) << " " << matrix.first.at<double>(0, 0) << std::endl;
                //ArucoDetector::solarPanelDetector(tags, matrix.first, matrix.first, robotPose);
            }
        }

        if (client.shouldStop() || stopRequested)
        {
            break;
        }
        std::cout << std::endl;
    }

    return 0;
}