#include "ArucoDetector.h"

ArucoDetector::ArucoDetector(const std::string& calibrationPath, const Team team, const bool headless) : headless(headless), team(team)
{
    // opencv 4.8
    // this->detector = cv::aruco::ArucoDetector(getPredefinedDictionary(cv::aruco::DICT_4X4_50), cv::aruco::DetectorParameters());
    // this->dictionary = cv::aruco::getPredefinedDictionary(cv::aruco::DICT_4X4_50);
    // this->parameters = cv::aruco::DetectorParameters();

    // 4.6
    this->dictionary = cv::aruco::getPredefinedDictionary(cv::aruco::DICT_4X4_50);
    this->parameters = cv::aruco::DetectorParameters::create();

    // TODO
    // Adjusting parameters based on specific needs
    parameters->adaptiveThreshConstant = true;
    parameters->minMarkerPerimeterRate = 0.03;
    parameters->maxMarkerPerimeterRate = 4.0;
    parameters->perspectiveRemoveIgnoredMarginPerCell = 0.13;
    parameters->polygonalApproxAccuracyRate = 0.03;

    this->readCameraParameters(calibrationPath);

    this->cam = new lccv::PiCamera;
    cam->options->video_width=1920;
    cam->options->video_height=1080;
    cam->options->framerate=5;
    cam->options->verbose=true;

    started = true;

    if (!headless)
    {
        cv::namedWindow("ArUco Detection", cv::WINDOW_NORMAL);
    }

    // auto whiteFlower = ArucoTag(36, "White flower", 20, FLOWER);
    // whiteFlower.setFlowerObjectRepresentation();
    // this->addArucoTag(whiteFlower);
    // auto purpleFlower = ArucoTag(13, "Purple flower", 20, FLOWER);
    // purpleFlower.setFlowerObjectRepresentation();
    // this->addArucoTag(purpleFlower);

    // this->addArucoTag(ArucoTag(47, "Solar panel", 50, SOLAR_PANEL));

    cam->startVideo();
}

ArucoDetector::~ArucoDetector()
{
    cam->stopVideo();
    cv::destroyAllWindows();
    delete cam;
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

std::pair<int, std::vector<std::pair<ArucoTag, std::pair<cv::Mat, cv::Mat>>>> ArucoDetector::detectArucoTags(std::vector<ArucoTag> tags)
{
    /*if (!started)
    {
        std::pair<int, std::vector<std::pair<ArucoTag, std::pair<cv::Mat, cv::Mat>>>> result;
        result.first = -2;
        return result;
    }*/

    if (tags.empty())
    {
        tags = this->arucoTags;
    }

    cv::Mat frame;
    cv::Mat frameNotRotated;
    cv::Mat frameDistored;
    cam->getVideoFrame(frameNotRotated, 1000);
    cv::flip(frameNotRotated, frame, -1);
    // cv::undistort(frameDistored, frame, cameraMatrix, distCoeffs);

    std::pair<int, std::vector<std::pair<ArucoTag, std::pair<cv::Mat, cv::Mat>>>> result;

    if (frame.empty()) {
        result.first = -2;
        return result;
    }

    // Convert frame to grayscale
    cv::Mat gray;
    cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);

    std::vector<int> markerIds;
    std::vector<std::vector<cv::Point2f>> markerCorners;

    // Preprocess with adaptive thresholding to handle varying lighting
    /*cv::Mat adaptiveThresh;
    cv::adaptiveThreshold(gray, adaptiveThresh, 255, cv::ADAPTIVE_THRESH_MEAN_C, cv::THRESH_BINARY, 11, 2);

    // Find contours in the thresholded image
    std::vector<std::vector<cv::Point>> contours;
    std::vector<cv::Vec4i> hierarchy;
    cv::findContours(adaptiveThresh, contours, hierarchy, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE);

    // Approximate contours to polygons
    std::vector<std::vector<cv::Point>> approxContours(contours.size());
    for (size_t i = 0; i < contours.size(); i++) {
        cv::approxPolyDP(cv::Mat(contours[i]), approxContours[i], 3, true); // Adjust epsilon as needed
    }

    // Use approximated contours to define regions of interest (ROIs)
    std::vector<cv::Rect> ROIs;
    for (const auto& contour : approxContours) {
        cv::Rect boundingRect = cv::boundingRect(contour);
        // Optionally, apply filtering based on ROI size or aspect ratio
        // For example:
        // if (boundingRect.area() > minArea && boundingRect.width / static_cast<double>(boundingRect.height) > minAspectRatio) {
        ROIs.push_back(boundingRect);
    }
    // Detect ArUco markers within defined ROIs
    for (const auto& roi : ROIs) {
        cv::Rect roiRect = roi & cv::Rect(0, 0, frame.cols, frame.rows); // Ensure the ROI is within the image boundaries
        cv::Mat roiFrame = frame(roiRect).clone(); // Extract ROI

        // Detect ArUco markers within the ROI
        std::vector<int> ids;
        std::vector<std::vector<cv::Point2f>> corners;
        cv::aruco::detectMarkers(roiFrame, this->dictionary, corners, ids, this->parameters);

        // Adjust marker corners to global coordinates
        for (auto& corner : corners) {
            for (auto& pt : corner) {
                pt.x += roiRect.x;
                pt.y += roiRect.y;
            }
        }

        // Merge results
        markerIds.insert(markerIds.end(), ids.begin(), ids.end());
        markerCorners.insert(markerCorners.end(), corners.begin(), corners.end());
    }*/
    // opencv 4.8
    // detector.detectMarkers(frame, markerCorners, markerIds);

    // 4.6
    cv::aruco::detectMarkers(frame, this->dictionary, markerCorners, markerIds, this->parameters);

    if (!markerIds.empty())
    {
        if (!headless)
        {
            cv::aruco::drawDetectedMarkers(frame, markerCorners, markerIds);
        }

        for (size_t i = 0; i < markerCorners.size(); i++)
        {
            int id = markerIds[i];

            if (std::find_if(tags.begin(), tags.end(), [id](const ArucoTag& tag) { return tag.id == id; }) == tags.end())
            {
                continue;
            }

            ArucoTag tag = *std::find_if(tags.begin(), tags.end(), [id](const ArucoTag& tag) { return tag.id == id; });

            cv::Mat rvec, tvec;

            try {
                solvePnP(tag.objectRepresenation, markerCorners.at(i), cameraMatrix, distCoeffs, rvec, tvec, false);
            } catch (const cv::Exception& e) {
                std::cerr << "Error: " << e.what() << std::endl;
                continue;
            }

            if (!headless)
            {
                drawFrameAxes(frame, cameraMatrix, distCoeffs, rvec, tvec, tag.length/2.f);
                // draw::drawCenterPoints(frame, markerCorners, 100);
            }

            // Convert rotation vector to rotation matrix
            cv::Mat rotationMatrix;
            cv::Rodrigues(rvec, rotationMatrix);

            // std::cout << rvec << std::endl;
            // std::cout << rotationMatrix << std::endl;

            // Extract Euler angles from the rotation matrix
            double roll, pitch, yaw;
            pitch = asin(rotationMatrix.at<double>(2, 0));
            roll = atan2(-rotationMatrix.at<double>(2, 1), rotationMatrix.at<double>(2, 2));
            yaw = atan2(-rotationMatrix.at<double>(1, 0), rotationMatrix.at<double>(0, 0));
            // Angles can be used to calculate the distance to the center of the flower.

            // std::cout << roll << " " << pitch << " " << yaw << std::endl << std::endl;

            cv::Mat rotaEuler = (cv::Mat_<double>(3, 1) << roll, pitch, yaw);

            // Apply the homogeneous transformation to tvec

            result.second.emplace_back(tag, std::make_pair(tvec, rotaEuler));
        }
    }

    std::sort(result.second.begin(), result.second.end(), [this](const std::pair<ArucoTag, std::pair<cv::Mat, cv::Mat>>& a, const std::pair<ArucoTag, std::pair<cv::Mat, cv::Mat>>& b)
    {
        return distanceBetweenRobotAndTagOnXY(a.second.first) > distanceBetweenRobotAndTagOnXY(b.second.first);
    });

    if (!headless)
    {
        cv::imshow("ArUco Detection", frame);
        if (cv::waitKey(10) == 27)
        {
            // Press 'Esc' to exit
            result.first = 1;
            return result;
        }
    }

    result.first = 0;
    return result;
}

/*void ArucoDetector::flowerDetector(const ArucoTag& tag, const cv::Mat& translationMatrix, const cv::Mat& rotationMatrix, Type::RobotPose* robotPose)
{
    constexpr double distanceToPot = 21;

    const double distanceXFlower = translationMatrix.at<double>(0, 0);// + (distanceToPot * sin(rotationMatrix.at<double>(1, 0)));
    const double distanceZFlower = translationMatrix.at<double>(2, 0);// + (distanceToPot * cos(rotationMatrix.at<double>(1, 0)));
    const double distanceFlower = distanceBetweenRobotAndTag(robotPose, translationMatrix);

    //std::cout << tag.name << " Pos : x: " << distanceXFlower << " z: " << distanceZFlower << " " << "distance: " << distanceFlower << std::endl;
}

void ArucoDetector::solarPanelDetector(const ArucoTag& tag, cv::Mat translationMatrix, const cv::Mat& rotationMatrix, Type::RobotPose* robotPose)
{
    std::cout << tag.name << " Pos : x: " << translationMatrix.at<double>(0, 0) << " z: " << translationMatrix.at<double>(2, 0) << " " << std::endl;
    const auto yaw = rotationMatrix.at<double>(2, 0);

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
*/