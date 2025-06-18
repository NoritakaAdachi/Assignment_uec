// visualize_reprojection.cpp
// g++ -std=c++17 visualize_reprojection.cpp -o visrep `pkg-config --cflags --libs opencv4`

#include <opencv2/opencv.hpp>
#include <iostream>
#include <iomanip>
#include <vector>

int main(int argc, char** argv)
{
    if (argc < 4) {
        std::cerr << "Usage: " << argv[0]
                  << " <camera.yaml> <pattern_cols> <pattern_rows> [<square_size_mm>=24] [<img_glob>=calib_img/*.png]\n";
        return EXIT_FAILURE;
    }

    const std::string calibFile   = argv[1];
    const int patCols            = std::stoi(argv[2]);
    const int patRows            = std::stoi(argv[3]);
    const float squareSize       = (argc >= 5) ? std::stof(argv[4]) : 24.0f;
    const std::string imgPattern = (argc >= 6) ? argv[5] : "calib_img/*.png";

    // 1. カメラパラメータを読み込む
    cv::FileStorage fs(calibFile, cv::FileStorage::READ);
    if (!fs.isOpened()) {
        std::cerr << "Cannot open calibration file: " << calibFile << '\n';
        return EXIT_FAILURE;
    }
    cv::Mat K, dist;
    fs["cameraMatrix"] >> K;
    fs["distCoeffs"]   >> dist;
    fs.release();

    if (K.empty() || dist.empty()) {
        std::cerr << "cameraMatrix or distCoeffs not found in file\n";
        return EXIT_FAILURE;
    }

    // 2. チェスボードの 3D 座標を用意
    const cv::Size patternSize(patCols, patRows);
    std::vector<cv::Point3f> objp;
    objp.reserve(patCols * patRows);
    for (int r = 0; r < patRows; ++r)
        for (int c = 0; c < patCols; ++c)
            objp.emplace_back(c * squareSize, r * squareSize, 0.0f);

    // 3. 対象画像を取得
    std::vector<cv::String> imageFiles;
    cv::glob(imgPattern, imageFiles, /*recursive=*/false);
    if (imageFiles.empty()) {
        std::cerr << "No images found for pattern: " << imgPattern << '\n';
        return EXIT_FAILURE;
    }

    double totalErr = 0.0;
    int    totalPts = 0;

    for (size_t idx = 0; idx < imageFiles.size(); ++idx) {
        cv::Mat img = cv::imread(imageFiles[idx]);
        if (img.empty()) {
            std::cerr << "Cannot read " << imageFiles[idx] << '\n';
            continue;
        }
        cv::Mat gray;
        cv::cvtColor(img, gray, cv::COLOR_BGR2GRAY);

        std::vector<cv::Point2f> corners;
        bool found = cv::findChessboardCorners(gray, patternSize, corners,
                      cv::CALIB_CB_ADAPTIVE_THRESH | cv::CALIB_CB_NORMALIZE_IMAGE);

        std::cout << std::setw(2) << idx << " : "
                  << (found ? "found" : "not found") << "  (" << imageFiles[idx] << ")\n";

        if (!found) continue;

        // サブピクセル精度に
        cv::cornerSubPix(gray, corners, {11,11}, {-1,-1},
                {cv::TermCriteria::EPS | cv::TermCriteria::COUNT, 30, 0.01});

        // 4. カメラ姿勢（rvec, tvec）を求める
        cv::Mat rvec, tvec;
        cv::solvePnP(objp, corners, K, dist, rvec, tvec);

        // 5. 再投影
        std::vector<cv::Point2f> proj;
        cv::projectPoints(objp, rvec, tvec, K, dist, proj);

        // 6. 誤差計算
        double err = cv::norm(corners, proj, cv::NORM_L2);
        totalErr += err * err;
        totalPts += static_cast<int>(corners.size());

        // 7. 描画：実測=緑，再投影=赤
        for (size_t i = 0; i < corners.size(); ++i) {
            cv::circle(img, corners[i], 4, {0,255,0}, 1, cv::LINE_AA); // 実測
            cv::circle(img, proj[i]   , 10, {0,0,255}, -1, cv::LINE_AA);// 再投影
        }

        // ファイル保存
        const std::string outName = "vis_" + std::to_string(idx) + ".png";
        cv::imwrite(outName, img);
        std::cout << "  -> saved " << outName << "  (RMS err=" << std::sqrt(err*err / corners.size()) << " px)\n";
    }

    if (totalPts > 0)
        std::cout << "\nGlobal RMS reprojection error = "
                  << std::sqrt(totalErr / totalPts) << " px\n";

    return EXIT_SUCCESS;
}
