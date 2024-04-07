#include <iostream>
#include <opencv2/opencv.hpp>

int main(int argc, char *argv[]) {

    cv::aruco::Dictionary AruCoDict = cv::aruco::getPredefinedDictionary(cv::aruco::DICT_4X4_50);

    int squaresX = 1920;
    int squaresY = 1080;
    float squareLength = 20;
    float markerLength = 10;

    cv::aruco::CharucoBoard board(cv::Size(squaresX, squaresY), squareLength, markerLength, AruCoDict);

    cv::VideoCapture cap(0);
    if (!cap.isOpened()) {
        std::cerr << "Error: Could not open the camera." << std::endl;
        return -1;
    }

    cv::Mat img, imgCopy;

    cv::aruco::DetectorParameters detectorParams = cv::aruco::DetectorParameters();

    cv::aruco::CharucoParameters charucoParams;

    cv::aruco::CharucoDetector detector(board, charucoParams, detectorParams);

    // Collect data from each frame
    std::vector<cv::Mat> allCharucoCorners;
    std::vector<cv::Mat> allCharucoIds;

    std::vector<std::vector<cv::Point2f>> allImagePoints;
    std::vector<std::vector<cv::Point3f>> allObjectPoints;

    std::vector<cv::Mat> allImages;
    cv::Size imageSize;

    while(cap.grab()) {
        cv::Mat image, imageCopy;
        cap.retrieve(image);

        std::vector<int> markerIds;
        std::vector<std::vector<cv::Point2f>> markerCorners, rejectedMarkers;
        cv::Mat currentCharucoCorners;
        cv::Mat currentCharucoIds;
        std::vector<cv::Point3f> currentObjectPoints;
        std::vector<cv::Point2f> currentImagePoints;

        // Detect ChArUco board
        detector.detectBoard(image, currentCharucoCorners, currentCharucoIds);

        // Draw results
        image.copyTo(imageCopy);
        if(!markerIds.empty()) {
            cv::aruco::drawDetectedMarkers(imageCopy, markerCorners);
        }

        if(currentCharucoCorners.total() > 3) {
            cv::aruco::drawDetectedCornersCharuco(imageCopy, currentCharucoCorners, currentCharucoIds);
        }

        putText(imageCopy, "Press 'c' to add current frame. 'ESC' to finish and calibrate",
                cv::Point(10, 20), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(255, 0, 0), 2);

        cv::imshow("out", imageCopy);

        // Wait for key pressed
        char key = static_cast<char>(cv::waitKey(10));

        if(key == 27) {
            break;
        }

        if(key == 'c' && currentCharucoCorners.total() > 3) {
            // Match image points
            board.matchImagePoints(currentCharucoCorners, currentCharucoIds, currentObjectPoints, currentImagePoints);

            if(currentImagePoints.empty() || currentObjectPoints.empty()) {
                std::cout << "Point matching failed, try again." << std::endl;
                continue;
            }

            std::cout << "Frame captured" << std::endl;

            allCharucoCorners.push_back(currentCharucoCorners);
            allCharucoIds.push_back(currentCharucoIds);
            allImagePoints.push_back(currentImagePoints);
            allObjectPoints.push_back(currentObjectPoints);
            allImages.push_back(image);

            imageSize = image.size();
        }
    }

    if(allCharucoCorners.size() < 4) {
        std::cerr << "Not enough corners for calibration" << std::endl;
        return 0;
    }

    cv::Mat cameraMatrix, distCoeffs;

    int calibrationFlags = 0;

    // Calibrate camera using ChArUco
    double repError = calibrateCamera(
        allObjectPoints, allImagePoints, imageSize,
        cameraMatrix, distCoeffs, cv::noArray(), cv::noArray(), cv::noArray(),
        cv::noArray(), cv::noArray(), calibrationFlags
    );

    std::cout << "Rep Error: " << repError << std::endl;

    cv::FileStorage fs("calibration_results.yaml", cv::FileStorage::WRITE);
    fs << "cameraMatrix" << cameraMatrix;
    fs << "distCoeffs" << distCoeffs;
    fs.release(); // Release the file

    return 0;
}