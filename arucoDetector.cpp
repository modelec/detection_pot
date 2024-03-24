#include "aruco/ArucoDetector.h"
#include "tcp/MyClient.h"

#include <QCoreApplication>
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
    QCoreApplication app(argc, argv);

    bool headless = false;

    for (int i = 0; i < argc; i++)
    {
        if (std::string(argv[i]) == "--help")
        {
            std::cout << "Usage: " << argv[0] << " <video capture device> <path/to/calibration_results.yaml>" << std::endl;
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
        std::cout << "Usage: " << argv[0] << " <video capture device> <path/to/calibration_results.yaml>" << std::endl;
        return 1;
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

    auto* robotPose = new Type::RobotPose{cv::Point3f(0, 0, 0), CV_PI/2};

    ArucoDetector detector(robotPose, calibrationPath, BLUE, cameraId, headless);

    auto whiteFlower = ArucoTag(36, "White_flower", 20, FLOWER);
    //whiteFlower.setFlowerObjectRepresentation();
    auto purpleFlower = ArucoTag(13, "Purple_flower", 20, FLOWER);
    //purpleFlower.setFlowerObjectRepresentation();

    auto solarPanel = ArucoTag(47, "Solar_panel", 50, SOLAR_PANEL);

    int code;

    MyClient client(robotPose, "127.0.0.1", 8080);

    client.start();

    client.sendMessage("request robotPose");

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

        for (auto [tags, matrix] : r.second)
        {
            if (tags.type == FLOWER)
            {
                constexpr double distanceToPot = 21;

                const double distanceXFlower = matrix.first.at<double>(0, 0);// + (distanceToPot * sin(rotationMatrix.at<double>(1, 0)));
                const double distanceZFlower = matrix.first.at<double>(2, 0);// + (distanceToPot * cos(rotationMatrix.at<double>(1, 0)));

                //std::cout << tags.name << " Pos : x: " << distanceXFlower << " z: " << distanceZFlower << " " << std::endl;

            } else if (tags.type == SOLAR_PANEL)
            {
                std::cout << tags.name << " Pos : x: " << matrix.first.at<double>(0, 0) << " z: " << matrix.first.at<double>(2, 0) << " " << std::endl;
                const auto yaw = matrix.second.at<double>(2, 0);

                const auto rotationBaseTable = (-yaw) + robotPose->theta;

                std::cout << " Rotation: " << rotationBaseTable * (180 / CV_PI) << std::endl;

                if (rotationBaseTable > 70 && rotationBaseTable <= 110)
                {
                    std::cout << "Mid" << std::endl;
                }
                else if (rotationBaseTable > 30 && rotationBaseTable <= 70)
                {
                    std::cout << "Blue side" << std::endl;
                }
                else if (rotationBaseTable > 110 && rotationBaseTable <= 150)
                {
                    std::cout << "Yellow side" << std::endl;
                }
                else
                {
                    std::cout << "Mid" << std::endl;
                }

                // BLUE => 90, YELLOW => -90
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

    return QCoreApplication::exec();
}