#include <rclcpp/rclcpp.hpp>
#include <onnxruntime_cxx_api.h>
#include <opencv2/opencv.hpp>

#include <array>
#include <iostream>
#include <string>
#include <vector>

#include "cv/onnx_helper.h"

struct Keypoint
{
    float x;
    float y;
    float score;
};

int main(int argc, char **argv)
{
    rclcpp::init(argc, argv);

    auto node = std::make_shared<rclcpp::Node>("human_detector");
    auto logger = node->get_logger();

    const char *model_path =
        "/home/steve/Desktop/drone6_ros/models/Movenet.onnx";

    Ort::Env env(ORT_LOGGING_LEVEL_WARNING, "human_detector");
    Ort::SessionOptions session_options;
    Ort::Session session(env, model_path, session_options);

    RCLCPP_INFO(logger, "Model loaded successfully");

    print_modelinfo(logger, session);

    std::string pipeline =
        "udpsrc port=50001 "
        "! h264parse "
        "! avdec_h264 "
        "! videoflip method=rotate-180 "
        "! videoconvert "
        "! appsink sync=false drop=true max-buffers=1";

    RCLCPP_INFO(logger, "Opening UDP H264 stream...");

    cv::VideoCapture cap(pipeline, cv::CAP_GSTREAMER);

    if (!cap.isOpened())
    {
        RCLCPP_ERROR(logger, "Failed to open stream");
        rclcpp::shutdown();
        return 1;
    }

    cv::Mat frame;

    while (rclcpp::ok())
    {
        if (!cap.read(frame) || frame.empty())
        {
            RCLCPP_WARN(logger, "No frame");
            continue;
        }

        cv::Mat resized;
        cv::resize(frame, resized, cv::Size(192, 192));

        cv::Mat rgb;
        cv::cvtColor(resized, rgb, cv::COLOR_BGR2RGB);

        rgb.convertTo(rgb, CV_32F);

        std::vector<float> input_values(1 * 3 * 192 * 192);

        for (int y = 0; y < 192; ++y)
        {
            for (int x = 0; x < 192; ++x)
            {
                cv::Vec3f pixel = rgb.at<cv::Vec3f>(y, x);

                input_values[0 * 192 * 192 + y * 192 + x] = pixel[0]; // R
                input_values[1 * 192 * 192 + y * 192 + x] = pixel[1]; // G
                input_values[2 * 192 * 192 + y * 192 + x] = pixel[2]; // B
            }
        }

        std::array<int64_t, 4> input_shape{1, 3, 192, 192};

        Ort::MemoryInfo memory_info = Ort::MemoryInfo::CreateCpu(
            OrtArenaAllocator,
            OrtMemTypeDefault);

        Ort::Value input_tensor = Ort::Value::CreateTensor<float>(
            memory_info,
            input_values.data(),
            input_values.size(),
            input_shape.data(),
            input_shape.size());

        const char *input_names[] = {"image"};
        const char *output_names[] = {"kpt_with_conf"};

        auto output_tensors = session.Run(
            Ort::RunOptions{nullptr},
            input_names,
            &input_tensor,
            1,
            output_names,
            1);

        float *output = output_tensors[0].GetTensorMutableData<float>();

        for (int i = 0; i < 17; ++i)
        {
            float y_norm = output[i * 3 + 0];
            float x_norm = output[i * 3 + 1];
            float conf = output[i * 3 + 2];

            RCLCPP_INFO(logger, "raw %02d: y=%.4f x=%.4f conf=%.4f", i, y_norm, x_norm, conf);

            if (conf < 0.3f)
                continue;

            int px = static_cast<int>(x_norm * frame.cols);
            int py = static_cast<int>(y_norm * frame.rows);

            cv::circle(
                frame,
                cv::Point(px, py),
                5,
                cv::Scalar(0, 255, 0),
                -1);
        }

        cv::imshow("MoveNet", frame);
        if (cv::waitKey(1) == 27)
        {
            break;
        }
    }

    cv::destroyAllWindows();
    rclcpp::shutdown();
    return 0;

    rclcpp::shutdown();
    return 0;
}