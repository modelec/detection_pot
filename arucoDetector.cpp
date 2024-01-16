#include <opencv2/opencv.hpp>
#include <opencv2/aruco.hpp>

#include "utils/utils.h"

int main(int argc, char *argv[])
{
    if (argc < 2) {
        std::cout << "Usage: " << argv[0] << " <video capture device>" << std::endl;
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

    int cameraId = std::stoi(argv[1]);

    if (cameraId < 0)
    {
        std::cerr << "Error: Camera ID must be a positive integer." << std::endl;
        return -1;
    }

    cv::VideoCapture cap(cameraId);

    if (!cap.isOpened()) {
        std::cerr << "Error opening camera." << std::endl;
        return -1;
    }

    if (!headless)
    {
        cv::namedWindow("ArUco Detection", cv::WINDOW_NORMAL);
    }

    cv::Mat cameraMatrix, distCoeffs;
    cv::FileStorage fs("../calibration_images/calibration_results.yaml", cv::FileStorage::READ);
    if (fs.isOpened()) {
        fs["cameraMatrix"] >> cameraMatrix;
        fs["distCoeffs"] >> distCoeffs;
        fs.release();
    } else {
        std::cerr << "Error reading calibration file." << std::endl << "See the calibration --help for more information." << std::endl;
        return -1;
    }

    float markerLength = 0.02;

    // Set coordinate system
    cv::Mat objPoints(4, 1, CV_32FC3);
    objPoints.ptr<cv::Vec3f>(0)[0] = cv::Vec3f(-markerLength/2.f, markerLength/2.f, 0);
    objPoints.ptr<cv::Vec3f>(0)[1] = cv::Vec3f(markerLength/2.f, markerLength/2.f, 0);
    objPoints.ptr<cv::Vec3f>(0)[2] = cv::Vec3f(markerLength/2.f, -markerLength/2.f, 0);
    objPoints.ptr<cv::Vec3f>(0)[3] = cv::Vec3f(-markerLength/2.f, -markerLength/2.f, 0);



    cv::aruco::DetectorParameters detectorParams = cv::aruco::DetectorParameters();
    cv::aruco::Dictionary dictionary = cv::aruco::getPredefinedDictionary(cv::aruco::DICT_4X4_50);

    // opencv 4.9
    cv::aruco::ArucoDetector detector(dictionary, detectorParams);

    while (true) {
        cv::Mat frame;
        cap >> frame;  // Capture frame from the camera

        if (frame.empty()) {
            std::cerr << "Error capturing frame." << std::endl;
            break;
        }

        cv::Mat gray;
        cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);

        std::vector<int> markerIds;
        std::vector<std::vector<cv::Point2f>> markerCorners;

        // cv::aruco::detectMarkers(frame, &dictionary, markerCorners, markerIds, &detectorParams);

        // opencv 4.9
        detector.detectMarkers(frame, markerCorners, markerIds);

        if (!markerIds.empty()) {
            std::cout << "Detected " << markerIds.size() << " markers." << std::endl;
            if (!headless)
            {
                cv::aruco::drawDetectedMarkers(frame, markerCorners, markerIds);
            }

            size_t nMarkers = markerCorners.size();

            for (size_t i = 0; i < nMarkers; i++) {
                cv::Mat rvec, tvec;

                solvePnP(objPoints, markerCorners.at(i), cameraMatrix, distCoeffs, rvec, tvec);



                if (!headless)
                {
                    drawFrameAxes(frame, cameraMatrix, distCoeffs, rvec, tvec, markerLength/2.f);
                    // draw::drawCenterPoints(frame, markerCorners, 100);
                }

                // Convert rotation vector to rotation matrix
                cv::Mat rotationMatrix;
                cv::Rodrigues(rvec, rotationMatrix);

                // Extract Euler angles from the rotation matrix
                double roll, pitch, yaw;
                pitch = asin(rotationMatrix.at<double>(2, 0));
                roll = atan2(-rotationMatrix.at<double>(2, 1), rotationMatrix.at<double>(2, 2));
                yaw = atan2(-rotationMatrix.at<double>(1, 0), rotationMatrix.at<double>(0, 0));

                // Convert angles from radians to degrees
                roll *= (180.0 / CV_PI);
                pitch *= (180.0 / CV_PI);
                yaw *= (180.0 / CV_PI);

                // Distance to ArUco marker (assuming tagLength is known)
                double distanceX = tvec.at<double>(0, 0);
                double distanceY = tvec.at<double>(2, 0);
                double distanceZ = tvec.at<double>(1, 0);

                // Print the distance and yaw angle
                // std::cout << "Distance to ArUco marker " << markerIds[i] << ": " << distance << " meters" << ", pitch : " << pitch << ", roll : " << roll << ", yaw : " << yaw << std::endl;
                std::cout << "Distance to ArUco marker " << markerIds[i] << ": " << distanceX << " " << distanceY << " " << distanceZ << std::endl;
            }
        }

        if (!headless)
        {
            cv::imshow("ArUco Detection", frame);
        }

        if (cv::waitKey(10) == 27)  // Press 'Esc' to exit
            break;
    }

    cap.release();
    cv::destroyAllWindows();

    return 0;
}