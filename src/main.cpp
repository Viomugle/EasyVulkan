#include "GlfwGeneral.hpp"
int main()
{
    if(!InitializedWindow({800,600}))
        return -1;
    while (!glfwWindowShouldClose(pWindow))
    {
        glfwPollEvents();
        TitleFps();
    }
    TerminateWindow();
    return 0;
}