# Locate Image On Screen
Locate an image on the screen and return its position using [OpenCV](https://github.com/opencv/opencv)

## Example of use
```cpp
#include "locateImageOnScreen.h"
#include <thread>
#include <chrono>

int main() {
    while (true) {
        POINT pos;
        if (locateImageOnScreen("image.png", pos, true, 0.8)) {
          SetCursorPos(pos.x, pos.y);
          mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
          mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }

    return 0;
}
```
