#include <opencv2/opencv.hpp>
#include <iostream>
#include <filesystem>
#include <vector>
#include <string>

int main() {
    const std::string image_dir = "calib_imgs";
    const cv::Size pattern_size(8, 5); // inner corners: 9x6 squares -> 8x5 corners
    const float square_size = 0.02f;   // 2 cm in meters

    std::vector<cv::Point3f> objp;
    for (int y = 0; y < pattern_size.height; y++) {
        for (int x = 0; x < pattern_size.width; x++) {
            objp.emplace_back(x * square_size, y * square_size, 0.0f);
        }
    }

    std::vector<std::vector<cv::Point3f>> obj_points;
    std::vector<std::vector<cv::Point2f>> img_points;

    cv::Size image_size;

    for (const auto& entry : std::filesystem::directory_iterator(image_dir)) {
        std::string filename = entry.path().string();

        cv::Mat img = cv::imread(filename);
        if (img.empty()) continue;

        image_size = img.size();

        cv::Mat gray;
        cv::cvtColor(img, gray, cv::COLOR_BGR2GRAY);

        std::vector<cv::Point2f> corners;
        bool found = cv::findChessboardCorners(gray, pattern_size, corners);

        std::cout << filename << " : " << (found ? "found" : "not found") << "\n";

        if (!found) continue;

        cv::cornerSubPix(
            gray,
            corners,
            cv::Size(11, 11),
            cv::Size(-1, -1),
            cv::TermCriteria(
                cv::TermCriteria::EPS + cv::TermCriteria::MAX_ITER,
                30,
                0.001
            )
        );

        obj_points.push_back(objp);
        img_points.push_back(corners);

        cv::drawChessboardCorners(img, pattern_size, corners, found);
        cv::imshow("Detected corners", img);
        cv::waitKey(0);
    }

    cv::destroyAllWindows();

    if (img_points.size() < 5) {
        std::cerr << "Not enough valid calibration images.\n";
        return 1;
    }

    cv::Mat K, dist;
    std::vector<cv::Mat> rvecs, tvecs;

    double rms = cv::calibrateCamera(
        obj_points,
        img_points,
        image_size,
        K,
        dist,
        rvecs,
        tvecs
    );

    std::cout << "\nValid images: " << img_points.size() << "\n";
    std::cout << "RMS reprojection error: " << rms << "\n\n";
    std::cout << "Camera matrix K:\n" << K << "\n\n";
    std::cout << "Distortion coefficients:\n" << dist << "\n";

    cv::FileStorage fs("./calibration/camera_calibration.yaml", cv::FileStorage::WRITE);
    fs << "image_width" << image_size.width;
    fs << "image_height" << image_size.height;
    fs << "camera_matrix" << K;
    fs << "distortion_coefficients" << dist;
    fs << "rms_reprojection_error" << rms;
    fs.release();

    std::cout << "\nSaved to camera_calibration.yaml\n";

    return 0;
}