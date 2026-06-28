cmake_minimum_required(VERSION 3.8)
project(cv)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# find dependencies
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)

find_package(OpenCV REQUIRED)
set(ONNXRUNTIME_ROOT
    ${CMAKE_CURRENT_SOURCE_DIR}/../../third_party/onnxruntime
)

# human detector
add_executable(human_detector
    src/human_detector.cpp
)
ament_target_dependencies(human_detector
    rclcpp
)
target_link_libraries(human_detector
    ${OpenCV_LIBS}
    ${ONNXRUNTIME_ROOT}/lib/libonnxruntime.so
)
target_include_directories(human_detector PRIVATE
    ${ONNXRUNTIME_ROOT}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
install(TARGETS
    human_detector
    DESTINATION lib/${PROJECT_NAME}
)
#end 

# test opencv installation with gstreamer support by opening udp stream from drone
add_executable(stream_viewer
    test/test_opencv.cpp
)
ament_target_dependencies(stream_viewer
    rclcpp
)

target_link_libraries(stream_viewer
    ${OpenCV_LIBS}
)

install(TARGETS
    stream_viewer
    DESTINATION lib/${PROJECT_NAME}
)
#end

# test ONNX runtime 
add_executable(test_onnx
    test/test_onnx.cpp
)

target_include_directories(test_onnx PRIVATE
    ${ONNXRUNTIME_ROOT}/include
)

target_link_libraries(test_onnx
    ${ONNXRUNTIME_ROOT}/lib/libonnxruntime.so
)

ament_target_dependencies(test_onnx
    rclcpp
)

install(TARGETS
    test_onnx
    DESTINATION lib/${PROJECT_NAME}
)
# end 



if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  # the following line skips the linter which checks for copyrights
  # comment the line when a copyright and license is added to all source files
  set(ament_cmake_copyright_FOUND TRUE)
  # the following line skips cpplint (only works in a git repo)
  # comment the line when this package is in a git repo and when
  # a copyright and license is added to all source files
  set(ament_cmake_cpplint_FOUND TRUE)
  ament_lint_auto_find_test_dependencies()
endif()

ament_package()
