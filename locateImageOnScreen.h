#pragma once
#include <windows.h>
#include <string>

// Locate an image on the screen and return its position
bool locateImageOnScreen(const std::string& imagePath, POINT& locatedPos, bool grayscale = true, float confidence = 0.9f);
