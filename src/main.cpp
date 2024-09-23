#include "GlfwGeneral.hpp"
#include "tools.hpp"

using namespace vulkan; //main.cpp里会写一堆vulkan命名空间下的类型，using命名空间以省事


int main() {
    if (!InitializeWindow({1280,720}))
        return -1;

    /*新增*/const auto& [renderPass, framebuffers] = easyVulkan::CreateRpwf_Screen();

    fence fence;
    semaphore semaphore_imageIsAvailable;
    semaphore semaphore_renderingIsOver;

    commandBuffer commandBuffer;
    commandPool commandPool(graphicsBase::Base().QueueFamilyIndex_Graphics(), VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
    commandPool.AllocateBuffers(commandBuffer);

    /*新增*/VkClearValue clearColor = { .color = { 1.f, 0.f, 0.f, 1.f } };//红色

    while (!glfwWindowShouldClose(pWindow)) {
        while (glfwGetWindowAttrib(pWindow, GLFW_ICONIFIED))
            glfwWaitEvents();

        graphicsBase::Base().SwapImage(semaphore_imageIsAvailable);
        //因为帧缓冲与所获取的交换链图像一一对应，获取交换链图像索引
        /*新增*/auto i = graphicsBase::Base().CurrentImageIndex();

        commandBuffer.Begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
        /*新增，开始渲染通道*/renderPass.CmdBegin(commandBuffer, framebuffers[i], { {}, windowSize }, clearColor);
        /*渲染命令，待填充*/
        /*新增，结束渲染通道*/renderPass.CmdEnd(commandBuffer);
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
