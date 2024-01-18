#include "ArucoDetector.h"

ArucoDetector::ArucoDetector(const Type::RobotPose pose, const std::string& calibrationPath, const int cameraId, const bool headless) : robotPose(pose), headless(headless)
{
    this->detector = cv::aruco::ArucoDetector(getPredefinedDictionary(cv::aruco::DICT_4X4_50), cv::aruco::DetectorParameters());
    this->transformationMatrix = (cv::Mat_<double>(4, 4) <<
        cos(pose.theta), 0, sin(pose.theta), pose.position.x,
        0, 1, 0, pose.position.y,
        -sin(pose.theta), 0, cos(pose.theta), pose.position.z,
        0, 0, 0, 1
    );
    this->readCameraParameters(calibrationPath);

    this->cap = cv::VideoCapture(cameraId);

    if (!cap.isOpened()) {
        std::cerr << "Error opening camera." << std::endl;
    }

    if (!headless)
    {
        cv::namedWindow("ArUco Detection", cv::WINDOW_NORMAL);
    }

    this->addArucoTag(ArucoTag(36, "White flower", 20, FLOWER));
    this->addArucoTag(ArucoTag(13, "Purple flower", 20, FLOWER));
    this->addArucoTag(ArucoTag(47, "Solar panel", 20, FLOWER));
}

ArucoDetector::ArucoDetector(const float x, const float y, const float z, const float theta, const std::string& calibrationPath, const int cameraId, const bool headless) : ArucoDetector(Type::RobotPose{cv::Point3f(x, y, z), theta}, calibrationPath, cameraId, headless)
{
}

ArucoDetector::~ArucoDetector()
{
    cap.release();
    cv::destroyAllWindows();
}


void ArucoDetector::readCameraParameters(const std::string& path)
{
    cv::FileStorage fs(path, cv::FileStorage::READ);
    if (fs.isOpened()) {
        fs["cameraMatrix"] >> this->cameraMatrix;
        fs["distCoeffs"] >> this->distCoeffs;
        fs.release();
    } else {
        std::cerr << "Error reading calibration file." << std::endl;
    }
}

void ArucoDetector::addArucoTag(const ArucoTag& tag)
{
    this->arucoTags.push_back(tag);
}

int ArucoDetector::detectArucoTags()
{
    cv::Mat frame;
    cap >> frame;  // Capture frame from the camera

    if (frame.empty()) {
        std::cerr << "Error capturing frame." << std::endl;
        return -2;
    }

    cv::Mat gray;
    cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);

    std::vector<int> markerIds;
    std::vector<std::vector<cv::Point2f>> markerCorners;

    // cv::aruco::detectMarkers(frame, &dictionary, markerCorners, markerIds, &detectorParams);

    // opencv 4.9
    detector.detectMarkers(frame, markerCorners, markerIds);

    if (!markerIds.empty())
    {
        std::cout << "Detected " << markerIds.size() << " markers." << std::endl;
        if (!headless)
        {
            cv::aruco::drawDetectedMarkers(frame, markerCorners, markerIds);
        }

        for (size_t i = 0; i < markerCorners.size(); i++) {
            int id = markerIds[i];

            if (std::find_if(arucoTags.begin(), arucoTags.end(), [id](const ArucoTag& tag) { return tag.id == id; }) == arucoTags.end())
            {
                continue;
            }

            ArucoTag tag = *std::find_if(arucoTags.begin(), arucoTags.end(), [id](const ArucoTag& tag) { return tag.id == id; });

            cv::Mat rvec, tvec;

            solvePnP(tag.objectRepresenation, markerCorners.at(i), cameraMatrix, distCoeffs, rvec, tvec);

            if (!headless)
            {
                drawFrameAxes(frame, cameraMatrix, distCoeffs, rvec, tvec, tag.length/2.f);
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
            // Angles can be used to calculate the distance to the center of the flower.

            cv::Mat rotaEuler = (cv::Mat_<double>(3, 1) << roll, pitch, yaw);

            // Apply the homogeneous transformation to tvec
            cv::Mat translat = (cv::Mat_<double>(4, 1) << tvec.at<double>(0, 0), tvec.at<double>(1, 0), tvec.at<double>(2, 0), 1);

            cv::Mat transformedTvec = (transformationMatrix * translat);

            if (tag.type == FLOWER)
            {
                flowerDetector(tag, transformedTvec, rotaEuler);
            } else if (tag.type == SOLAR_PANEL)
            {
                solarPanelDetector(tag, transformedTvec, rotaEuler);
            }
        }
    }

    if (!headless)
    {
        cv::imshow("ArUco Detection", frame);
    }

    if (cv::waitKey(10) == 27)  // Press 'Esc' to exit
        return 1;

    return 0;
}

void ArucoDetector::flowerDetector(const ArucoTag& tag, const cv::Mat& translationMatrix, const cv::Mat& rotationMatrix)
{
    constexpr double distanceToPot = 21;

    const double distanceXFlower = translationMatrix.at<double>(0, 0) + (distanceToPot * sin(rotationMatrix.at<double>(1, 0)));
    const double distanceZFlower = translationMatrix.at<double>(2, 0) + (distanceToPot * cos(rotationMatrix.at<double>(1, 0)));

    std::cout << tag.name << " Pos : x: " << distanceXFlower << " z: " << distanceZFlower << " " << std::endl;
}

void ArucoDetector::solarPanelDetector(ArucoTag tag, cv::Mat translationMatrix, cv::Mat rotationMatrix)
{
    std::cout << tag.name << " Pos : x: " << translationMatrix.at<double>(0, 0) << " z: " << translationMatrix.at<double>(2, 0) << " " << std::endl;
}

