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
    cam.options->photo_width=2028;
    cam.options->photo_height=1520;
    cam.options->verbose=true;
    for(int i=0;i<30;i++){
        std::cout<<i<<std::endl;
        if(!cam.capturePhoto(image)){
            std::cout<<"Camera error"<<std::endl;
        }

        cv::imwrite("calibration_images/"+std::to_string(i)+".jpg", image);

        sleep(1000);
    }

    return 0;
}