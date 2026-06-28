#include <rclcpp/rclcpp.hpp>
#include <onnxruntime_cxx_api.h>

#include <iostream>
#include <string>
#include <vector>

static std::string shape_to_string(const std::vector<int64_t> &shape)
{
    std::string s = "[";
    for (size_t i = 0; i < shape.size(); ++i)
    {
        s += std::to_string(shape[i]);
        if (i + 1 < shape.size())
            s += ", ";
    }
    s += "]";
    return s;
}

void print_modelinfo(rclcpp::Logger logger, const Ort::Session& session)
{
    size_t input_count = session.GetInputCount();
    size_t output_count = session.GetOutputCount();
    Ort::AllocatorWithDefaultOptions allocator;

    for (size_t i = 0; i < input_count; ++i)
    {
        auto name = session.GetInputNameAllocated(i, allocator);

        auto type_info = session.GetInputTypeInfo(i);
        auto tensor_info = type_info.GetTensorTypeAndShapeInfo();

        auto shape = tensor_info.GetShape();
        auto elem_type = tensor_info.GetElementType();

        RCLCPP_INFO(
            logger,
            "Input %zu: name=%s, shape=%s, elem_type=%d",
            i,
            name.get(),
            shape_to_string(shape).c_str(),
            static_cast<int>(elem_type));
    }

    RCLCPP_INFO(logger, "Outputs: %zu", output_count);

    for (size_t i = 0; i < output_count; ++i)
    {
        auto name = session.GetOutputNameAllocated(i, allocator);

        auto type_info = session.GetOutputTypeInfo(i);
        auto tensor_info = type_info.GetTensorTypeAndShapeInfo();

        auto shape = tensor_info.GetShape();
        auto elem_type = tensor_info.GetElementType();

        RCLCPP_INFO(
            logger,
            "Output %zu: name=%s, shape=%s, elem_type=%d",
            i,
            name.get(),
            shape_to_string(shape).c_str(),
            static_cast<int>(elem_type));
    }
}