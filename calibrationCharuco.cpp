#include <iostream>
#include <opencv2/opencv.hpp>
#include <lccv.hpp>
#include <thread>

#include <opencv2/aruco/charuco.hpp>

int main(int argc, char *argv[]) {

    cv::Ptr<cv::aruco::Dictionary> AruCoDict = cv::aruco::getPredefinedDictionary(cv::aruco::DICT_4X4_50);

    int squaresX = 5;
    int squaresY = 7;
    float squareLength = 0.0338f;
    float markerLength = 0.0167f;
    std::string outputFile = "camera_calibration.yml";

    // create charuco board object
    cv::Ptr<cv::aruco::CharucoBoard> charucoboard =
            cv::aruco::CharucoBoard::create(squaresX, squaresY, squareLength, markerLength, AruCoDict);
    cv::Ptr<cv::aruco::Board> board = charucoboard.staticCast<cv::aruco::Board>();

    lccv::PiCamera* cam = new lccv::PiCamera;
    cam->options->video_width=1920;
    cam->options->video_height=1080;
    cam->options->framerate=5;
    cam->options->verbose=true;
    cv::namedWindow("Video",cv::WINDOW_NORMAL);
    cam->startVideo();

    cv::Ptr<cv::aruco::DetectorParameters> detectorParams = cv::aruco::DetectorParameters::create();

    // cv::aruco::CharucoParameters charucoParams;

    // cv::aruco::CharucoDetector detector(board, charucoParams, detectorParams);

    // Collect data from each frame
    // std::vector<cv::Mat> allCharucoCorners;
    // std::vector<cv::Mat> allCharucoIds;
    //
    // std::vector<std::vector<cv::Point2f>> allImagePoints;
    // std::vector<std::vector<cv::Point3f>> allObjectPoints;
    //
    // std::vector<cv::Mat> allImages;
    // cv::Size imageSize;

    std::vector< std::vector< std::vector< cv::Point2f > > > allCorners;
    std::vector< std::vector< int > > allIds;
    std::vector< cv::Mat > allImgs;
    cv::Size imgSize;

    char key;
    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    while(key != 27) {
        cv::Mat image, imageCopy, imgNotRotated;
        if(!cam->getVideoFrame(imgNotRotated,1000)){
            std::cout<<"Timeout error"<<std::endl;
            continue;
        }

        cv::flip(imgNotRotated, image, -1);

        std::vector< int > ids;
        std::vector< std::vector< cv::Point2f > > corners, rejected;

        cv::aruco::detectMarkers(image, AruCoDict, corners, ids, detectorParams, rejected);

        cv::Mat currentCharucoCorners, currentCharucoIds;
        if(!ids.empty())
            cv::aruco::interpolateCornersCharuco(corners, ids, image, charucoboard, currentCharucoCorners,
                                             currentCharucoIds);
        // draw results
        image.copyTo(imageCopy);
        if(!ids.empty()) cv::aruco::drawDetectedMarkers(imageCopy, corners);

        if(currentCharucoCorners.total() > 0)
            cv::aruco::drawDetectedCornersCharuco(imageCopy, currentCharucoCorners, currentCharucoIds);

        putText(imageCopy, "Press 'c' to add current frame. 'ESC' to finish and calibrate",
                cv::Point(10, 20), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(255, 0, 0), 2);

        imshow("Video", imageCopy);
        key = (char)cv::waitKey(10);
        if(key == 'c' && !ids.empty()) {
            std::cout << "Frame captured" << std::endl;
            allCorners.push_back(corners);
            allIds.push_back(ids);
            allImgs.push_back(image);
            imgSize = image.size();
        }
    }


    if(allIds.empty()) {
        std::cerr << "Not enough captures for calibration" << std::endl;
        return 0;
    }

    cv::Mat cameraMatrix, distCoeffs;
    std::vector<cv::Mat> rvecs, tvecs;
    double repError;

    int calibrationFlags = 0;

    // Calibrate camera using ChArUco
    // double repError = calibrateCamera(
    //     allObjectPoints, allImagePoints, imageSize,
    //     cameraMatrix, distCoeffs, cv::noArray(), cv::noArray(), cv::noArray(),
    //     cv::noArray(), cv::noArray(), calibrationFlags
    // );
    // prepare data for calibration

    std::vector< std::vector< cv::Point2f > > allCornersConcatenated;
    std::vector< int > allIdsConcatenated;
    std::vector< int > markerCounterPerFrame;
    markerCounterPerFrame.reserve(allCorners.size());
    for(unsigned int i = 0; i < allCorners.size(); i++) {
        markerCounterPerFrame.push_back((int)allCorners[i].size());
        for(unsigned int j = 0; j < allCorners[i].size(); j++) {
            allCornersConcatenated.push_back(allCorners[i][j]);
            allIdsConcatenated.push_back(allIds[i][j]);
        }
    }

    double arucoRepErr;
    arucoRepErr = cv::aruco::calibrateCameraAruco(allCornersConcatenated, allIdsConcatenated,
                                              markerCounterPerFrame, board, imgSize, cameraMatrix,
                                              distCoeffs, cv::noArray(), cv::noArray(), calibrationFlags);


    /*int nFrames = (int)allCorners.size();
    std::vector< cv::Mat > allCharucoCorners;
    std::vector< cv::Mat > allCharucoIds;
    std::vector< cv::Mat > filteredImages;
    allCharucoCorners.reserve(nFrames);
    allCharucoIds.reserve(nFrames);

    for(int i = 0; i < nFrames; i++) {
        // interpolate using camera parameters
        cv::Mat currentCharucoCorners, currentCharucoIds;
        cv::aruco::interpolateCornersCharuco(allCorners[i], allIds[i], allImgs[i], charucoboard,
                                         currentCharucoCorners, currentCharucoIds, cameraMatrix,
                                         distCoeffs);

        allCharucoCorners.push_back(currentCharucoCorners);
        allCharucoIds.push_back(currentCharucoIds);
        filteredImages.push_back(allImgs[i]);
    }

    if(allCharucoCorners.size() < 4) {
        std::cerr << "Not enough corners for calibration" << std::endl;
        return 0;
    }

    // calibrate camera using charuco
    repError =
        cv::aruco::calibrateCameraCharuco(allCharucoCorners, allCharucoIds, charucoboard, imgSize,
                                      cameraMatrix, distCoeffs, rvecs, tvecs, calibrationFlags);*/

    // std::cout << "Rep Error: " << repError << std::endl;
    std::cout << "Rep Error Aruco: " << arucoRepErr << std::endl;
    std::cout << "Calibration saved to " << outputFile << std::endl;

    std::cout << "Rep Error: " << arucoRepErr << std::endl;

    cv::FileStorage fs(outputFile, cv::FileStorage::WRITE);
    fs << "cameraMatrix" << cameraMatrix;
    fs << "distCoeffs" << distCoeffs;
    fs.release(); // Release the file
    cam->stopVideo();
    cv::destroyAllWindows();

    return 0;
}
