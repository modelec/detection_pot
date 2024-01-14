#include <iostream>
#include <filesystem>
#include <opencv2/opencv.hpp>

int main(int argc, char *argv[])
{

    if (argc < 2) {
        std::cout << "Usage: " << argv[0] << " <directory>" << std::endl;
        return 1;
    }

    // Set the chessboard size (number of inner corners in width and height)
    cv::Size chessboardSize(9, 6);

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

    const std::string pathName = argv[1];

    try {
        for (const auto& entry : std::filesystem::directory_iterator(pathName)) {
            if (entry.is_regular_file()) {
                // Check if the file has a ".jpg" or ".png" extension
                std::string fileExtension = entry.path().extension().string();
                if (fileExtension == ".jpg" || fileExtension == ".png") {
                    std::cout << entry.path() << std::endl;

                    // Load the calibration image
                    cv::Mat image = cv::imread(entry.path().string());

                    imgSize = image.size();

                    // Convert the image to grayscale
                    cv::Mat gray;
                    cv::cvtColor(image, gray, cv::COLOR_BGR2GRAY);

                    // Find chessboard corners
                    std::vector<cv::Point2f> corners;
                    bool found = cv::findChessboardCorners(gray, chessboardSize, corners);

                    if (found) {
                        cv::cornerSubPix(gray, corners, cv::Size(11, 11), cv::Size(-1, -1),
                                         cv::TermCriteria(cv::TermCriteria::EPS + cv::TermCriteria::COUNT, 30, 0.1));

                        imagePoints.push_back(corners);
                        objectPoints.push_back(worldPoints);
                    }
                }
            }
        }
    } catch (const std::filesystem::filesystem_error& e) {
        std::cerr << "Error accessing the directory: " << e.what() << std::endl;
    }

    cv::Mat cameraMatrix, distCoeffs;
    std::vector<cv::Mat> rvecs, tvecs;

    calibrateCamera(objectPoints, imagePoints, imgSize,
                        cameraMatrix, distCoeffs, rvecs, tvecs);

    cv::FileStorage fs(pathName + "/calibration_results.yaml", cv::FileStorage::WRITE);
    fs << "cameraMatrix" << cameraMatrix;
    fs << "distCoeffs" << distCoeffs;
    fs.release(); // Release the file

    return 0;

}