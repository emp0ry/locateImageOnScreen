#include "locateImageOnScreen.h"
#include <opencv2/opencv.hpp>

bool locateImageOnScreen(const std::string& imagePath, POINT& locatedPos, bool grayscale, float confidence) {
    // Validate confidence threshold (must be between 0 and 1)
    if (confidence < 0.0f || confidence > 1.0f) {
        std::cerr << "Invalid confidence threshold. Must be between 0.0f and 1.0f\n";
        return false;
    }

    // Set the process to be DPI-aware to handle high-resolution displays correctly
    SetProcessDPIAware();

    // Load the target image (either in grayscale or color based on the parameter)
    cv::Mat target = cv::imread(imagePath, grayscale ? cv::IMREAD_GRAYSCALE : cv::IMREAD_COLOR);
    if (target.empty()) {
        std::cerr << "Error: Failed to load target image\n";
        return false;
    }

    // Retrieve the dimensions of the virtual screen (includes multiple monitors)
    const int screenWidth = GetSystemMetrics(SM_CXVIRTUALSCREEN);
    const int screenHeight = GetSystemMetrics(SM_CYVIRTUALSCREEN);
    const int screenLeft = GetSystemMetrics(SM_XVIRTUALSCREEN);
    const int screenTop = GetSystemMetrics(SM_YVIRTUALSCREEN);

    // Ensure the target image is not larger than the screen dimensions
    if (target.cols > screenWidth || target.rows > screenHeight) {
        std::cerr << "Error: Target image is larger than the screen dimensions\n";
        return false;
    }

    // Create device contexts for capturing the screen
    HDC hdc = GetDC(nullptr); // Get the device context for the entire screen
    HDC memDC = CreateCompatibleDC(hdc); // Create a memory device context (off-screen buffer)
    HBITMAP hBitmap = CreateCompatibleBitmap(hdc, screenWidth, screenHeight); // Create a bitmap to store the screen capture
    HGDIOBJ oldObj = SelectObject(memDC, hBitmap); // Select the bitmap into the memory device context

    // Capture the screen and store it in hBitmap
    if (!BitBlt(memDC, 0, 0, screenWidth, screenHeight, hdc, screenLeft, screenTop, SRCCOPY)) {
        std::cerr << "Error: Screen capture failed\n";
        SelectObject(memDC, oldObj);
        DeleteObject(hBitmap);
        DeleteDC(memDC);
        ReleaseDC(nullptr, hdc);
        return false;
    }

    // Retrieve bitmap information
    BITMAP bmpInfo;
    GetObject(hBitmap, sizeof(BITMAP), &bmpInfo);

    // Calculate the stride (row alignment) for proper memory access
    const int stride = ((bmpInfo.bmWidth * bmpInfo.bmBitsPixel + 31) / 32) * 4;
    std::vector<BYTE> buffer(stride * bmpInfo.bmHeight); // Allocate buffer to store raw pixel data

    // Copy bitmap data into the buffer
    GetBitmapBits(hBitmap, static_cast<LONG>(buffer.size()), buffer.data());

    // Create an OpenCV matrix from the raw pixel data
    cv::Mat screenshot(bmpInfo.bmHeight, bmpInfo.bmWidth, CV_8UC4, buffer.data(), stride);

    // Convert from BGRA to grayscale or BGR based on the input flag
    cv::cvtColor(screenshot, screenshot, grayscale ? cv::COLOR_BGRA2GRAY : cv::COLOR_BGRA2BGR);

    // Clean up GDI objects
    SelectObject(memDC, oldObj);
    DeleteObject(hBitmap);
    DeleteDC(memDC);
    ReleaseDC(nullptr, hdc);

    // Perform template matching to find the target image within the screenshot
    cv::Mat result;
    try {
        cv::matchTemplate(screenshot, target, result, cv::TM_CCOEFF_NORMED);
    }
    catch (const cv::Exception& e) {
        std::cerr << "Error: Template matching failed: " << e.what() << '\n';
        return false;
    }

    // Find the best match location
    double maxVal;
    cv::Point maxLoc;
    cv::minMaxLoc(result, nullptr, &maxVal, nullptr, &maxLoc);

    // Check if the best match exceeds the confidence threshold
    if (maxVal >= confidence) {
        std::cout << maxVal << std::endl;
        locatedPos.x = maxLoc.x + target.cols / 2 + screenLeft; // Adjust for the actual screen position
        locatedPos.y = maxLoc.y + target.rows / 2 + screenTop;
        return true;
    }

    return false; // No match found above the confidence threshold
}
