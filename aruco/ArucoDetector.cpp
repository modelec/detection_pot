#include "ArucoDetector.h"

ArucoDetector::ArucoDetector(const Type::RobotPose& pose, const std::string& calibrationPath, const Team team, const int cameraId, const bool headless) : robotPose(pose), headless(headless), team(team)
{
    // opencv 4.8
    // this->detector = cv::aruco::ArucoDetector(getPredefinedDictionary(cv::aruco::DICT_4X4_50), cv::aruco::DetectorParameters());
    // this->dictionary = cv::aruco::getPredefinedDictionary(cv::aruco::DICT_4X4_50);
    // this->parameters = cv::aruco::DetectorParameters();

    // 4.6
    this->dictionary = cv::aruco::getPredefinedDictionary(cv::aruco::DICT_4X4_50);


    this->transformationMatrix = (cv::Mat_<double>(4, 4) <<
        cos(pose.theta), 0, sin(pose.theta), pose.position.x,
        0, 1, 0, pose.position.y,
        -sin(pose.theta), 0, cos(pose.theta), pose.position.z,
        0, 0, 0, 1
    );
    this->readCameraParameters(calibrationPath);

    // this->cap = cv::VideoCapture(cameraId);
    this->cam = new lccv::PiCamera;
    cam->options->video_width=1920;
    cam->options->video_height=1080;
    cam->options->framerate=10;
    cam->options->verbose=true;

/*    if (!cap.isOpened()) {
        std::cerr << "Error opening camera." << std::endl;
    } else
    {
        started = true;
    }*/
    started = true;

    if (!headless)
    {
        cv::namedWindow("ArUco Detection", cv::WINDOW_NORMAL);
    }

    auto whiteFlower = ArucoTag(36, "White flower", 20, FLOWER);
    whiteFlower.setFlowerObjectRepresentation();
    this->addArucoTag(whiteFlower);
    auto purpleFlower = ArucoTag(13, "Purple flower", 20, FLOWER);
    purpleFlower.setFlowerObjectRepresentation();
    this->addArucoTag(purpleFlower);

    this->addArucoTag(ArucoTag(47, "Solar panel", 50, SOLAR_PANEL));

    cam->startVideo();
}

ArucoDetector::ArucoDetector(const float x, const float y, const float z, const float theta, const std::string& calibrationPath, const Team team, const int cameraId, const bool headless) : ArucoDetector(Type::RobotPose{cv::Point3f(x, y, z), theta}, calibrationPath, team, cameraId, headless)
{
}

ArucoDetector::~ArucoDetector()
{
    cam->stopVideo();
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
    cam->getVideoFrame(frame, 1000);
    // cap >> frame;  // Capture frame from the camera

    std::pair<int, std::vector<std::pair<ArucoTag, std::pair<cv::Mat, cv::Mat>>>> result;

    if (frame.empty()) {
        result.first = -2;
        return result;
    }

    cv::Mat gray;
    cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);

    std::vector<int> markerIds;
    std::vector<std::vector<cv::Point2f>> markerCorners;

    // 4.6
    cv::aruco::detectMarkers(frame, this->dictionary, markerCorners, markerIds);

    // opencv 4.8
    // detector.detectMarkers(frame, markerCorners, markerIds);

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

            result.second.emplace_back(tag, std::make_pair(transformedTvec, rotaEuler));
        }
    }

    std::sort(result.second.begin(), result.second.end(), [this](const std::pair<ArucoTag, std::pair<cv::Mat, cv::Mat>>& a, const std::pair<ArucoTag, std::pair<cv::Mat, cv::Mat>>& b)
    {
        return distanceBetweenRobotAndTag(robotPose, a.second.first) < distanceBetweenRobotAndTag(robotPose, b.second.first);
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

void ArucoDetector::flowerDetector(const ArucoTag& tag, const cv::Mat& translationMatrix, const cv::Mat& rotationMatrix, const Type::RobotPose& robotPose, int serverSocket)
{
    constexpr double distanceToPot = 21;

    const double distanceXFlower = translationMatrix.at<double>(0, 0);// + (distanceToPot * sin(rotationMatrix.at<double>(1, 0)));
    const double distanceZFlower = translationMatrix.at<double>(2, 0);// + (distanceToPot * cos(rotationMatrix.at<double>(1, 0)));
    const double distanceFlower = distanceBetweenRobotAndTag(robotPose, translationMatrix);

    std::cout << tag.name << " Pos : x: " << distanceXFlower << " z: " << distanceZFlower << " " << "distance: " << distanceFlower << std::endl;
    std::string data = std::to_string(distanceXFlower) + ";" + std::to_string(distanceZFlower) + ";" + std::to_string(distanceFlower);
    std::cout << "Data: " << data << std::endl;
    sendData(serverSocket, data);
}

void ArucoDetector::solarPanelDetector(const ArucoTag& tag, cv::Mat translationMatrix, const cv::Mat& rotationMatrix, const Type::RobotPose& robotPose)
{
    std::cout << tag.name << " Pos : x: " << translationMatrix.at<double>(0, 0) << " z: " << translationMatrix.at<double>(2, 0) << " " << std::endl;
    const auto yaw = rotationMatrix.at<double>(2, 0);

    const auto rotationBaseTable = (-yaw) + robotPose.theta;

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

int ArucoDetector::startServer() {
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1) {
        std::cerr << "Error: Could not create socket." << std::endl;
        return -1;
    }
    sockaddr_in serverAddress{};
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(8080);
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress));
    listen(serverSocket, 5);
    return serverSocket;
}

void ArucoDetector::stopServer(int serverSocket) {
    close(serverSocket);
}

void ArucoDetector::setNonBlocking(int sockfd) {
    int flags = fcntl(sockfd, F_GETFL, 0);
    if (flags == -1) {
        // Handle error
        perror("fcntl F_GETFL");
        return;
    }
    if (fcntl(sockfd, F_SETFL, flags | O_NONBLOCK) == -1) {
        // Handle error
        perror("fcntl F_SETFL O_NONBLOCK");
        return;
    }
}

void ArucoDetector::sendData(int serverSocket, const std::string& data) {
    int clientSocket = accept(serverSocket, nullptr, nullptr);
    if(clientSocket == -1 && errno != EWOULDBLOCK && errno != EAGAIN) {
        perror("accept");
        return;
    }
    if(clientSocket != -1) {
        setNonBlocking(clientSocket);
        send(clientSocket, data.c_str(), data.size(), 0);
        close(clientSocket);
    }
}

