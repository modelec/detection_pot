#include <iostream>
#include <filesystem>
#include <opencv2/opencv.hpp>

int main(int argc, char *argv[])
{

    for (int i = 0; i < argc; i++) {
        if (std::string(argv[i]) == "--help")
        {
            std::cout << "Usage: " << argv[0] << " <directory>" << std::endl;
            std::cout << "directory: The directory containing the calibration images." << std::endl;
            return 0;
        }
    }

    if (argc < 2) {
        std::cout << "Usage: " << argv[0] << " <directory>" << std::endl;
        return 1;
    }

    // Set the chessboard size (number of inner corners in width and height)
    cv::Size chessboardSize(6, 9);

    cv::Size imgSize;

    // Create vectors to store the detected chessboard corners and corresponding image points
    std::vector<std::vector<cv::Point3f>> objectPoints; // 3D world points
    std::vector<std::vector<cv::Point2f>> imagePoints;  // 2D image points

    // Generate the 3D world points for the chessboard corners
    std::vector<cv::Point3f> worldPoints;
    for (int i = 0; i < chessboardSize.height; ++i) {
        for (int j = 0; j < chessboardSize.width; ++j) {
            worldPoints.emplace_back(j, i, 0); // Assuming the chessboard lies in the XY plane (Z=0)
        }
    }

    std::string imagesDirectory = argv[1];
    std::vector<cv::String> imageFiles;
    cv::glob(imagesDirectory + "/*.jpg", imageFiles);

    if (imageFiles.empty()) {
        std::cerr << "Error: No calibration images found in the specified directory." << std::endl;
        return -1;
    }

    for (const auto& imageFile : imageFiles) {
        std::cout << "Processing image: " << imageFile << std::endl;

        // Load the image
        cv::Mat img = cv::imread(imageFile);

        imgSize = img.size();

        // Convert the image to grayscale
        cv::Mat gray;
        cv::cvtColor(img, gray, cv::COLOR_BGR2GRAY);

        // Find chessboard corners
        std::vector<cv::Point2f> corners;

        if (findChessboardCorners(gray, chessboardSize, corners)) {
            // Refine corner locations
            cv::cornerSubPix(gray, corners, cv::Size(11, 11), cv::Size(-1, -1),
                             cv::TermCriteria(cv::TermCriteria::EPS + cv::TermCriteria::MAX_ITER, 30, 0.001));

            // Store object and image points
            objectPoints.push_back(worldPoints);
            imagePoints.push_back(corners);
        } else {
            std::cerr << "Warning: Chessboard pattern not found in image: " << imageFile << std::endl;
        }
    }

    cv::Mat cameraMatrix, distCoeffs;
    std::vector<cv::Mat> rvecs, tvecs;

    calibrateCamera(objectPoints, imagePoints, imgSize,
                        cameraMatrix, distCoeffs, rvecs, tvecs);

    cv::FileStorage fs(imagesDirectory + "/calibration_results.yaml", cv::FileStorage::WRITE);
    fs << "cameraMatrix" << cameraMatrix;
    fs << "distCoeffs" << distCoeffs;
    fs.release(); // Release the file

    return 0;

}