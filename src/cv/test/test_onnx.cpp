#include <rclcpp/rclcpp.hpp>
#include <onnxruntime_cxx_api.h>

#include <array>
#include <iostream>
#include <string>
#include <vector>

static std::string shape_to_string(const std::vector<int64_t>& shape)
{
    std::string s = "[";
    for (size_t i = 0; i < shape.size(); ++i) {
        s += std::to_string(shape[i]);
        if (i + 1 < shape.size()) s += ", ";
    }
    s += "]";
    return s;
}

int main(int argc, char **argv)
{
    rclcpp::init(argc, argv);

    auto node = std::make_shared<rclcpp::Node>("test_onnx");
    auto logger = node->get_logger();

    const char* model_path =
        "/home/steve/Desktop/drone6_ros/models/Movenet.onnx";

    Ort::Env env(ORT_LOGGING_LEVEL_WARNING, "drone6_onnx");
    Ort::SessionOptions session_options;
    Ort::Session session(env, model_path, session_options);

    RCLCPP_INFO(logger, "Model loaded successfully");

    std::array<int64_t, 4> input_shape = {1, 3, 192, 192};
    std::vector<float> input_tensor_values(1 * 3 * 192 * 192, 0.0f);

    Ort::MemoryInfo memory_info =
        Ort::MemoryInfo::CreateCpu(
            OrtArenaAllocator,
            OrtMemTypeDefault
        );

    Ort::Value input_tensor =
        Ort::Value::CreateTensor<float>(
            memory_info,
            input_tensor_values.data(),
            input_tensor_values.size(),
            input_shape.data(),
            input_shape.size()
        );

    const char* input_names[] = {"image"};
    const char* output_names[] = {"kpt_with_conf"};

    auto output_tensors = session.Run(
        Ort::RunOptions{nullptr},
        input_names,
        &input_tensor,
        1,
        output_names,
        1
    );

    RCLCPP_INFO(logger, "Inference ran successfully");

    float* output = output_tensors[0].GetTensorMutableData<float>();

    auto output_info =
        output_tensors[0].GetTensorTypeAndShapeInfo();

    auto output_shape = output_info.GetShape();

    RCLCPP_INFO(
        logger,
        "Output shape: %s",
        shape_to_string(output_shape).c_str()
    );

    for (int i = 0; i < 17; ++i) {
        float y = output[i * 3 + 0];
        float x = output[i * 3 + 1];
        float conf = output[i * 3 + 2];

        RCLCPP_INFO(
            logger,
            "Keypoint %02d: y=%.4f x=%.4f conf=%.4f",
            i, y, x, conf
        );
    }

    rclcpp::shutdown();
    return 0;
}