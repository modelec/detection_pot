#include <opencv2/opencv.hpp>

int main() {
    cv::aruco::Dictionary dictionary = cv::aruco::getPredefinedDictionary(cv::aruco::DICT_4X4_50);
    cv::aruco::CharucoBoard board(cv::Size(5, 7), 0.04f, 0.02f, dictionary);
    cv::Mat boardImage;
    board.generateImage(cv::Size(2480, 3508), boardImage, 10, 1);
    cv::imwrite("BoardImage.jpg", boardImage);

    return 0;
}