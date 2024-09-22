#include "GlfwGeneral.hpp"
#include "tools.hpp"

using namespace vulkan; //main.cpp里会写一堆vulkan命名空间下的类型，using命名空间以省事

int main() {
    if (!InitializeWindow({1280,720}))
        return -1;

    fence fence; //以非置位状态创建栅栏
    semaphore semaphore_imageIsAvailable;
    semaphore semaphore_renderingIsOver;

    commandBuffer commandBuffer;
    commandPool commandPool(graphicsBase::Base().QueueFamilyIndex_Graphics(), VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
    commandPool.AllocateBuffers(commandBuffer);

    while (!glfwWindowShouldClose(pWindow)) {
        while (glfwGetWindowAttrib(pWindow, GLFW_ICONIFIED))
            glfwWaitEvents();

        graphicsBase::Base().SwapImage(semaphore_imageIsAvailable);

        commandBuffer.Begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
        /*渲染命令，待填充*/
        commandBuffer.End();

        graphicsBase::Base().SubmitCommandBuffer_Graphics(commandBuffer, semaphore_imageIsAvailable, semaphore_renderingIsOver, fence);
        graphicsBase::Base().PresentImage(semaphore_renderingIsOver);

        glfwPollEvents();
        TitleFps();

        fence.WaitAndReset();
    }
    TerminateWindow();
    return 0;
}
