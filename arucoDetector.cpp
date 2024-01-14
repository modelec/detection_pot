#include <opencv2/opencv.hpp>
#include <opencv2/aruco.hpp>

int main()
{
    cv::VideoCapture cap(2);  // Open default camera (change the argument if using a different camera)

    if (!cap.isOpened()) {
        std::cerr << "Error opening camera." << std::endl;
        return -1;
    }

    cv::namedWindow("ArUco Detection", cv::WINDOW_NORMAL);

    cv::Mat cameraMatrix, distCoeffs;
    cv::FileStorage fs("../calibration_images/calibration_results.yaml", cv::FileStorage::READ);
    if (fs.isOpened()) {
        fs["cameraMatrix"] >> cameraMatrix;
        fs["distCoeffs"] >> distCoeffs;
        fs.release();
    } else {
        std::cerr << "Error reading calibration file." << std::endl;
        return -1;
    }

    float markerLength = 0.006;

    // Set coordinate system
    cv::Mat objPoints(4, 1, CV_32FC3);
    objPoints.ptr<cv::Vec3f>(0)[0] = cv::Vec3f(-markerLength/2.f, markerLength/2.f, 0);
    objPoints.ptr<cv::Vec3f>(0)[1] = cv::Vec3f(markerLength/2.f, markerLength/2.f, 0);
    objPoints.ptr<cv::Vec3f>(0)[2] = cv::Vec3f(markerLength/2.f, -markerLength/2.f, 0);
    objPoints.ptr<cv::Vec3f>(0)[3] = cv::Vec3f(-markerLength/2.f, -markerLength/2.f, 0);



    cv::aruco::DetectorParameters detectorParams = cv::aruco::DetectorParameters();
    cv::aruco::Dictionary dictionary = cv::aruco::getPredefinedDictionary(cv::aruco::DICT_4X4_50);
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
        detector.detectMarkers(frame, markerCorners, markerIds);

        if (!markerIds.empty()) {
            cv::aruco::drawDetectedMarkers(frame, markerCorners, markerIds);

            size_t nMarkers = markerCorners.size();

            for (size_t i = 0; i < nMarkers; i++) {
                cv::Mat rvec, tvec;

                solvePnP(objPoints, markerCorners.at(i), cameraMatrix, distCoeffs, rvec, tvec);
                drawFrameAxes(frame, cameraMatrix, distCoeffs, rvec, tvec, markerLength/2.f);

                // Extract rotation matrix from rotation vector
                cv::Mat R;
                Rodrigues(rvec, R);

                // Calculate Euler angles (in degrees)
                cv::Mat euler;
                Rodrigues(R, euler);

                // Access elements of the Euler angles matrix
                double yaw = euler.at<double>(2, 0) * (180.0 / CV_PI);

                // Distance to ArUco marker (assuming tagLength is known)
                double distance = tvec.at<double>(2, 0);

                // Print the distance and yaw angle
                std::cout << "Distance to ArUco marker " << markerIds[i] << ": " << distance << " meters" << ", " << yaw << " degrees" << std::endl;
            }
        }

        cv::imshow("ArUco Detection", frame);

        if (cv::waitKey(30) == 27)  // Press 'Esc' to exit
            break;
    }

    cap.release();
    cv::destroyAllWindows();

    return 0;
}