//
// Created by 24510 on 2024/8/21.
//

#ifndef GLFWGENERAL_HPP
#define GLFWGENERAL_HPP
#include "VKBase.h"
#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"
#include <format>
namespace vulkan
{
GLFWwindow* pWindow;
GLFWmonitor*  pMonitor;
const char * windowTitle = "Vulkan";
bool InitializedWindow(VkExtent2D size,bool fullScreen=false,bool isResizable=true,bool limitFrameRate=true)
{
    if (!glfwInit()) {
        std::cout << std::format("[ InitializeWindow ] ERROR\nFailed to initialize GLFW!\n");
        return false;
    }
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, isResizable);
    uint32_t extensionCount=0;
    const char** extensionNames;
    extensionNames= glfwGetRequiredInstanceExtensions(&extensionCount);
    if(!extensionNames)
    {
        std::cout<<std::format("[ InitializeWindow ] ERROR\nFailed to get required instance extensions!\n");
        glfwTerminate();
        return false;
    }
    for(size_t i=0;i<extensionCount;i++)
    {
        graphicsBase::Base().AddInstanceExtension(extensionNames[i]);
    }
    graphicsBase::Base().AddDeviceExtension(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
    pMonitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* pMode = glfwGetVideoMode(pMonitor);
    pWindow = fullScreen ?
        glfwCreateWindow(pMode->width, pMode->height, windowTitle, pMonitor, nullptr) :
        glfwCreateWindow(size.width, size.height, windowTitle, nullptr, nullptr);
    if (!pWindow) {
        std::cout << std::format("[ InitializeWindow ]\nFailed to create a glfw window!\n");
        glfwTerminate();
        return false;
    }

    return true;
}
void TerminateWindow()
{
    //TODO
    glfwTerminate();
}
void TitleFps()
{
    static double time0 = glfwGetTime();
    static double time1;
    static double dt;
    static int dframe = -1;
    static std::stringstream info;
    time1 = glfwGetTime();
    dframe++;
    if ((dt = time1 - time0) >= 1) {
        info.precision(1);
        info << windowTitle << "    " << std::fixed << dframe / dt << " FPS";
        glfwSetWindowTitle(pWindow, info.str().c_str());
        info.str("");//别忘了在设置完窗口标题后清空所用的stringstream
        time0 = time1;
        dframe = 0;
    }
}
}


#endif //GLFWGENERAL_HPP
