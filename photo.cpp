#include <lccv.hpp>
#include <opencv2/opencv.hpp>

void sleep(int ms){
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

int main()
{
    cv::Mat image;
    lccv::PiCamera cam;
    //cam.options->width=4056;
    //cam.options->height=3040;
    cam.options->photo_width=1920;
    cam.options->photo_height=1080;
    cam.options->framerate=10;
    cam.options->verbose=true;

    cam.startVideo();

    int nbPhoto = 20;

    for(int i=0;i<nbPhoto;i++){
        std::cout<<i<<std::endl;
        if (!cam.getVideoFrame(image, 1000)) {
            std::cerr << "Error getting frame from camera." << std::endl;
            return -1;
        } else {
            cv::imwrite("calibration_images/"+std::to_string(i)+".jpg", image);

            sleep(1000);
        }
    }
    cam.stopVideo();

    return 0;
}