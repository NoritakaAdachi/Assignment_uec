// Modern OpenCV camera calibration (C++17)
// Compile example:
// g++ -std=c++17 camera_calibration.cpp -o calib `pkg-config --cflags --libs opencv4`

#include <opencv2/opencv.hpp>
#include <iostream>
#include <iomanip>
#include <vector>
#include <string>

int main() {
    constexpr int IMAGE_NUM  = 24;   // number of calibration images
    constexpr int PAT_ROW    = 7;    // chessboard rows
    constexpr int PAT_COL    = 10;   // chessboard cols
    constexpr float CHESS_SIZE = 24.0f; // square size [mm]

    const cv::Size patternSize(PAT_COL, PAT_ROW);

    // Prepare object points for a single view (z = 0 plane)
    std::vector<cv::Point3f> singleObjectPoints;
    singleObjectPoints.reserve(PAT_ROW * PAT_COL);
    for (int r = 0; r < PAT_ROW; ++r)
        for (int c = 0; c < PAT_COL; ++c)
            singleObjectPoints.emplace_back(c * CHESS_SIZE, r * CHESS_SIZE, 0.0f);

    std::vector<std::vector<cv::Point3f>> objectPoints; // 3‑D points in world coord.
    std::vector<std::vector<cv::Point2f>> imagePoints;  // 2‑D points in image plane
    std::vector<cv::String> imageFiles;                 // image file paths

    // Build file list and duplicate object points for each image
    objectPoints.assign(IMAGE_NUM, singleObjectPoints);
    for (int i = 0; i < IMAGE_NUM; ++i) {
        char buf[32];
        std::snprintf(buf, sizeof(buf), "calib_img/%02d.jpg", i);
        imageFiles.emplace_back(buf);
    }

    cv::Size imageSize; // filled after first image load

    // Detect corners in each image
    for (int i = 0; i < IMAGE_NUM; ++i) {
        cv::Mat src = cv::imread(imageFiles[i]);
        if (src.empty()) {
            std::cerr << "Cannot load image file: " << imageFiles[i] << '\n';
            return EXIT_FAILURE;
        }
        if (i == 0) imageSize = src.size();

        std::vector<cv::Point2f> corners;
        bool found = cv::findChessboardCorners(src, patternSize, corners,
                        cv::CALIB_CB_ADAPTIVE_THRESH | cv::CALIB_CB_NORMALIZE_IMAGE);
        std::cout << std::setw(2) << std::setfill('0') << i << "..." << (found ? "ok" : "fail") << '\n';
        if (!found) return EXIT_FAILURE; // stop if any image fails

        cv::Mat gray;
        cv::cvtColor(src, gray, cv::COLOR_BGR2GRAY);
        cv::cornerSubPix(gray, corners, cv::Size(11, 11), cv::Size(-1, -1),
                         cv::TermCriteria(cv::TermCriteria::EPS | cv::TermCriteria::COUNT, 30, 0.01));

        imagePoints.push_back(corners);

        // Draw and show corners for visual confirmation
        cv::drawChessboardCorners(src, patternSize, corners, found);
        //cv::imshow("Calibration", src);
        cv::waitKey(0);
    }
    cv::destroyAllWindows();

    // Calibrate camera
    cv::Mat cameraMatrix, distCoeffs;
    std::vector<cv::Mat> rvecs, tvecs;
    double rms = cv::calibrateCamera(objectPoints, imagePoints, imageSize,
                                     cameraMatrix, distCoeffs, rvecs, tvecs);

    std::cout << "\nRMS reprojection error = " << rms << '\n';

    // Save parameters to YAML (or XML by changing extension)
    cv::FileStorage fs("camera.yaml", cv::FileStorage::WRITE);
    fs << "cameraMatrix" << cameraMatrix;
    fs << "distCoeffs"   << distCoeffs;
    if (!rvecs.empty()) fs << "rotation"    << rvecs[0];
    if (!tvecs.empty()) fs << "translation" << tvecs[0];
    fs.release();

    std::cout << "Calibration data written to camera.yaml\n";
    return EXIT_SUCCESS;
}
