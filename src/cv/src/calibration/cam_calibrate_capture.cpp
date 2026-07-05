#include <rclcpp/rclcpp.hpp>
#include <opencv2/opencv.hpp>

#include <filesystem>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

int main(int argc, char **argv)
{
    rclcpp::init(argc, argv);

    auto logger = rclcpp::get_logger("capture_calib");

    const std::string output_dir = "calib_imgs";
    std::filesystem::create_directories(output_dir);

    std::string pipeline =
        "udpsrc port=50001 "
        "! h264parse "
        "! avdec_h264 "
        "! videoflip method=rotate-180 "
        "! videoconvert "
        "! appsink sync=false drop=true max-buffers=1";

    RCLCPP_INFO(logger, "Opening UDP H264 stream...");

    cv::VideoCapture cap(pipeline, cv::CAP_GSTREAMER);

    if (!cap.isOpened()) {
        RCLCPP_ERROR(logger, "Could not open UDP H264 stream");
        rclcpp::shutdown();
        return 1;
    }

    int count = 0;

    while (rclcpp::ok()) {
        cv::Mat frame;
        cap >> frame;

        if (frame.empty()) {
            RCLCPP_WARN(logger, "Empty frame");
            continue;
        }

        cv::imshow("Calibration capture - SPACE save, q/ESC quit", frame);

        int key = cv::waitKey(1);

        if (key == 27 || key == 'q') {
            break;
        }

        if (key == ' ') {
            std::ostringstream filename;
            filename << output_dir << "/img_"
                     << std::setw(3) << std::setfill('0') << count
                     << ".jpg";

            if (cv::imwrite(filename.str(), frame)) {
                RCLCPP_INFO(logger, "Saved %s", filename.str().c_str());
                count++;
            } else {
                RCLCPP_ERROR(logger, "Failed to save image");
            }
        }
    }

    cap.release();
    cv::destroyAllWindows();
    rclcpp::shutdown();
    return 0;
}