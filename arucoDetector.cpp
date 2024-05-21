#include "aruco/ArucoDetector.h"
#include "tcp/MyClient.h"

#include <Modelec/CLParser.h>

#include <iostream>
#include <thread>
#include <atomic>
#include <optional>
#include <csignal>

std::atomic<bool> shouldStop = false;

void signalHandler( int signum ) {
    shouldStop = true;
}

int main(int argc, char *argv[])
{
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);
    // Settup argument parser

    bool headless = false;

    bool stopRequested = false;

    CLParser clParser(argc, argv);

    if (clParser.hasOption("--help"))
    {
        std::cout << "Usage: " << argv[0] << "<path/to/calibration_results.yaml> <port>" << std::endl;
        std::cout << "video capture device: The ID of the video capture device to use. Usually 0 for the built-in camera." << std::endl;
        std::cout << "path/to/calibration_results.yaml: The path to the calibration results file." << std::endl;
        std::cout << "to run the program in headless mode, add the --headless flag." << std::endl;
        return 0;
    }

    if (clParser.hasOption("headless")) {
        std::cout << "Running in headless mode." << std::endl;
        headless = true;
    }

    int port = clParser.getOption<int>("port", 8080);

    auto host = clParser.getOption("host", "127.0.0.1");

    std::optional<std::string> calibrationPath = clParser.getOption<std::string>("calib-file");

    if (!calibrationPath.has_value()) {
        std::cout << "Usage: " << argv[0] << "<path/to/calibration_results.yaml> <port>" << std::endl;
        return 1;
    }

    ArucoDetector detector(calibrationPath.value(), BLUE, headless);

    auto whiteFlower = ArucoTag(36, "White_flower", 18.3, FLOWER);
    // whiteFlower.setFlowerObjectRepresentation();
    auto purpleFlower = ArucoTag(13, "Purple_flower", 18.3, FLOWER);
    // purpleFlower.setFlowerObjectRepresentation();

    auto solarPanel = ArucoTag(47, "Solar_panel", 36.8, SOLAR_PANEL);

    int code;

    MyClient client(host, port);

    client.start();

    client.sendMessage("aruco;strat;ready;1");

    usleep(500'000);

    while (true) {

        std::pair<int, std::vector<std::pair<ArucoTag, std::pair<cv::Mat, cv::Mat>>>> r = std::make_pair(-1, std::vector<std::pair<ArucoTag, std::pair<cv::Mat, cv::Mat>>>());

        try {
            r = detector.detectArucoTags({whiteFlower, purpleFlower, solarPanel});
        } catch (const std::exception& ex) {
            std::cerr << "Error: " << ex.what() << std::endl;
            return -1;
        }

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
                std::cout << tags.id << " " << tags.name << " " << matrix.first.at<double>(2, 0) << " " << matrix.first.at<double>(0, 0) << " " << matrix.second.at<double>(0, 0) << " " << matrix.second.at<double>(1, 0) << " " << matrix.second.at<double>(2, 0) << std::endl;
                //ArucoDetector::flowerDetector(tags, matrix.first, matrix.first, robotPose);
            }
            else if (tags.type == SOLAR_PANEL)
            {
                // std::cout << tags.id << " " << tags.name << " " << matrix.first.at<double>(2, 0) << " " << matrix.first.at<double>(0, 0) << std::endl;
                //ArucoDetector::solarPanelDetector(tags, matrix.first, matrix.first, robotPose);
            }
        }

        if (client.shouldStop() || stopRequested || shouldStop)
        {
            break;
        }
        std::cout << std::endl;
    }

    return 0;
}