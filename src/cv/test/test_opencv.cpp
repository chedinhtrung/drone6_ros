#include <rclcpp/rclcpp.hpp>
#include <opencv2/opencv.hpp>

int main(int argc, char **argv)
{
    rclcpp::init(argc, argv);

    auto node = std::make_shared<rclcpp::Node>("stream_viewer");
    auto logger = node->get_logger();

    std::string pipeline =
        "udpsrc port=50001 "
        "! h264parse "
        "! avdec_h264 "
        "! videoconvert "
        "! appsink sync=false drop=true max-buffers=1";

    RCLCPP_INFO(logger, "Opening UDP H264 stream...");

    cv::VideoCapture cap(pipeline, cv::CAP_GSTREAMER);

    if (!cap.isOpened()) {
        RCLCPP_ERROR(logger, "Failed to open stream");
        rclcpp::shutdown();
        return 1;
    }

    cv::Mat frame;

    while (rclcpp::ok()) {
        if (!cap.read(frame) || frame.empty()) {
            RCLCPP_WARN(logger, "No frame");
            continue;
        }

        cv::imshow("Drone6 H264 Stream", frame);

        if (cv::waitKey(1) == 27) {
            break;
        }
    }

    cv::destroyAllWindows();
    rclcpp::shutdown();
    return 0;
}