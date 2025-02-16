#include "locateImageOnScreen.h"
#include <opencv2/opencv.hpp>

bool locateImageOnScreen(const std::string& imagePath, POINT& locatedPos, bool grayscale, float confidence) {
    // Load the target image
    cv::Mat target = cv::imread(imagePath, grayscale ? cv::IMREAD_GRAYSCALE : cv::IMREAD_COLOR);
    if (target.empty()) {
        std::cerr << "Error: Unable to load image.\n";
        return false;
    }

    // Get the virtual screen dimensions (all monitors combined)
    int screenWidth = GetSystemMetrics(SM_CXVIRTUALSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYVIRTUALSCREEN);
    int screenLeft = GetSystemMetrics(SM_XVIRTUALSCREEN);
    int screenTop = GetSystemMetrics(SM_YVIRTUALSCREEN);

    // Capture the entire virtual screen
    HDC hdc = GetDC(NULL);
    HDC memDC = CreateCompatibleDC(hdc);
    HBITMAP hBitmap = CreateCompatibleBitmap(hdc, screenWidth, screenHeight);
    SelectObject(memDC, hBitmap);
    BitBlt(memDC, 0, 0, screenWidth, screenHeight, hdc, screenLeft, screenTop, SRCCOPY);

    // Convert the bitmap to a cv::Mat
    cv::Mat screenshot(screenHeight, screenWidth, CV_8UC4);
    GetBitmapBits(hBitmap, screenWidth * screenHeight * 4, screenshot.data);
    cv::cvtColor(screenshot, screenshot, grayscale ? cv::COLOR_BGRA2GRAY : cv::COLOR_BGRA2BGR);

    // Release resources
    DeleteObject(hBitmap);
    DeleteDC(memDC);
    ReleaseDC(NULL, hdc);

    // Perform template matching
    cv::Mat result;
    cv::matchTemplate(screenshot, target, result, cv::TM_CCOEFF_NORMED);

    // Find the best match location
    double minVal, maxVal;
    cv::Point minLoc, maxLoc;
    cv::minMaxLoc(result, &minVal, &maxVal, &minLoc, &maxLoc);

    if (maxVal > confidence) {
        locatedPos = { maxLoc.x + target.cols / 2 + screenLeft, maxLoc.y + target.rows / 2 + screenTop };
        return true;
    }

    return false;
}
