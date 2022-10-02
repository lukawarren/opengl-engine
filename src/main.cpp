#include "window.h"
#include <iostream>

int main()
{
    Window window("nameless", 640, 480);
    while (window.update())
    {
        std::cout << " ";
    }
    return 0;
}