#include <thread>
#include <opencv2/opencv.hpp>

void sleep(int ms){
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

int main()
{
    auto cap = cv::VideoCapture(0);

    int nbPhoto = 20;

    for(int i=0;i<nbPhoto;i++){
        std::cout<<i<<std::endl;
        cv::Mat image;
        cap >> image;
        if(!image.empty()){
            cv::imwrite("../calibration_images/"+std::to_string(i)+".jpg", image);

            sleep(100);
        }
    }
    cap.release();

    return 0;
}