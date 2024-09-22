//
// Created by 24510 on 2024/8/21.
//

#ifndef VKBASE_H
#define VKBASE_H


#ifndef NDEBUG
#define ENABLE_DEBUG_MESSENGER true
#else
#define ENABLE_DEBUG_MESSENGER false
#endif

#include "EasyVulkanStart.h"
#include "tools.hpp"
#include <vulkan/vulkan.h>
#include "vulkan/vulkan_core.h"



namespace vulkan {


    constexpr VkExtent2D defaultWindowSize = {1280, 720};

    class graphicsBase {
        static graphicsBase singleton;

        graphicsBase() = default;

        graphicsBase(const graphicsBase &&) = delete;

        ~graphicsBase() {
            if (!instance)
                return;
            if (device) {
                WaitIdle();
                if (swapchain) {
                    for (auto &i: callbacks_destroySwapchain)
                        i();
                    for (auto &i: swapchainImageViews) {
                        if (i)
                            vkDestroyImageView(device, i, nullptr);
                        vkDestroySwapchainKHR(device, swapchain, nullptr);
                    }
                    for (auto &i: callbacks_destroyDevice)
                        i();
                    vkDestroyDevice(device, nullptr);
                }
                if (surface) {
                    vkDestroySurfaceKHR(instance, surface, nullptr);
                }
                if (debugMessenger) {
                    PFN_vkDestroyDebugReportCallbackEXT DestroyDebugUtilsMessenger = reinterpret_cast<PFN_vkDestroyDebugReportCallbackEXT>(vkGetInstanceProcAddr(
                            instance, "vkDestroyDebugUtilsMessengerEXT"));
                    if (DestroyDebugUtilsMessenger) {
                        DestroyDebugUtilsMessenger(instance, debugMessenger, nullptr);
                    }
                }
                vkDestroyInstance(instance, nullptr);
            }
        }

    public:
        static graphicsBase &Base() {
            return singleton;
        }

        void Terminate() {
            this->~graphicsBase();
            instance = VK_NULL_HANDLE;
            physicalDevice = VK_NULL_HANDLE;
            device = VK_NULL_HANDLE;
            surface = VK_NULL_HANDLE;
            swapchain = VK_NULL_HANDLE;
            swapchainImages.resize(0);
            swapchainImageViews.resize(0);
            swapchainCreateInfo = {};
            debugMessenger = VK_NULL_HANDLE;

        }

        VkResult WaitIdle() const {
            VkResult result = vkDeviceWaitIdle(device);
            if (result)
                std::cout << std::format("[ graphicsBase ] ERROR\nFailed to wait for device idle!\nError code: {}\n",
                                         int32_t(result));
            return result;
        }

        uint32_t CurrentImageIndex() const { return currentImageIndex; }

        result_t SwapImage(VkSemaphore semaphore_imageIsAvailable) {
            //销毁旧交换链（若存在）
            if (swapchainCreateInfo.oldSwapchain &&
                swapchainCreateInfo.oldSwapchain != swapchain) {
                vkDestroySwapchainKHR(device, swapchainCreateInfo.oldSwapchain, nullptr);
                swapchainCreateInfo.oldSwapchain = VK_NULL_HANDLE;
            }
            //获取交换链图像索引
            while (VkResult result = vkAcquireNextImageKHR(device, swapchain, UINT64_MAX, semaphore_imageIsAvailable,
                                                           VK_NULL_HANDLE, &currentImageIndex))
                switch (result) {
                    case VK_SUBOPTIMAL_KHR:
                    case VK_ERROR_OUT_OF_DATE_KHR:
                        if (VkResult result = RecreateSwapchain())
                            return result;
                        break; //注意重建交换链后仍需要获取图像，通过break递归，再次执行while的条件判定语句
                    default:
                        outStream << std::format(
                                "[ graphicsBase ] ERROR\nFailed to acquire the next image!\nError code: {}\n",
                                int32_t(result));
                        return result;
                }
            return VK_SUCCESS;
        }


    private:
        VkInstance instance;
        std::vector<const char *> instanceLayers;
        std::vector<const char *> instanceExtensions;
        std::vector<void (*)()> callbacks_createSwapchain;
        std::vector<void (*)()> callbacks_destroySwapchain;
        std::vector<void (*)()> callbacks_createDevice;
        std::vector<void (*)()> callbacks_destroyDevice;

        uint32_t currentImageIndex = 0;


        VkResult RecreateDevice(VkDeviceCreateFlags flags = 0) {
            if (VkResult result = WaitIdle())
                return result;
            if (swapchain) {
                for (auto &i: callbacks_destroySwapchain)
                    i();
                for (auto &i: swapchainImageViews) {
                    if (i)
                        vkDestroyImageView(device, i, nullptr);
                }
                swapchainImageViews.resize(0);
                vkDestroySwapchainKHR(device, swapchain, nullptr);
                swapchain = VK_NULL_HANDLE;
                swapchainCreateInfo = {};
            }
            for (auto &i: callbacks_destroyDevice)
                i();
            if (device)
                vkDestroyDevice(device, nullptr);
            device = VK_NULL_HANDLE;
            return CreateDevice(flags);
        }

        static void AddLayerOrExtension(std::vector<const char *> &container, const char *name) {
            for (auto &i: container) {
                if (!strcmp(name, i))
                    return;
            }
            container.push_back(name);
        }

    public:
        void AddCallback_CreateSwapchain(void(*function)()) {
            callbacks_createSwapchain.push_back(function);
        }

        result_t SubmitCommandBuffer_Graphics(VkSubmitInfo &submitInfo, VkFence fence = VK_NULL_HANDLE) const {
            submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
            VkResult result = vkQueueSubmit(queue_graphics, 1, &submitInfo, fence);
            if (result)
                outStream
                        << std::format("[ graphicsBase ] ERROR\nFailed to submit the command buffer!\nError code: {}\n",
                                       int32_t(result));
            return result;
        }

        result_t SubmitCommandBuffer_Graphics(VkCommandBuffer commandBuffer,
                                              VkSemaphore semaphore_imageIsAvailable = VK_NULL_HANDLE,
                                              VkSemaphore semaphore_renderingIsOver = VK_NULL_HANDLE,
                                              VkFence fence = VK_NULL_HANDLE,
                                              VkPipelineStageFlags waitDstStage_imageIsAvailable = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT) const {
            VkSubmitInfo submitInfo = {
                    .commandBufferCount = 1,
                    .pCommandBuffers = &commandBuffer
            };
            if (semaphore_imageIsAvailable) {
                submitInfo.waitSemaphoreCount = 1;
                submitInfo.pWaitSemaphores = &semaphore_imageIsAvailable;
                submitInfo.pWaitDstStageMask = &waitDstStage_imageIsAvailable;
            }
            if (semaphore_renderingIsOver) {
                submitInfo.signalSemaphoreCount = 1;
                submitInfo.pSignalSemaphores = &semaphore_renderingIsOver;
            }
            return SubmitCommandBuffer_Graphics(submitInfo, fence);
        }

        result_t SubmitCommandBuffer_Compute(VkSubmitInfo& submitInfo,VkFence fence=VK_NULL_HANDLE)const
        {
            submitInfo.sType=VK_STRUCTURE_TYPE_SUBMIT_INFO;
            VkResult result=vkQueueSubmit(queue_compute,1,&submitInfo,fence);
            if(result)
                outStream<<std::format("[ graphicsBase ] ERROR\nFailed to submit the command buffer!\nError code: {}\n",int32_t(result));
            return result;
        }
        result_t SubmitCommandBuffer_Compute(VkCommandBuffer commandBuffer,VkFence fence=VK_NULL_HANDLE)cosnt
        {
            VkSubmitInfo submitInfo={
                    .commandBufferCount=1,
                    .pCommandBuffers=&commandBuffer
            };
            return SubmitCommandBuffer_Compute(submitInfo,fence);
        }

        result_t PresentImage(VkPresentInfoKHR& presentInfo)
        {
            presentInfo.sType=VK_STRUCTURE_TYPE_INFO_KHR;
            switch(VkResult result=PFN_vkQueuePresentKHR(queue_presentation,&presentInfo))
            {
                case VK_SUCCESS:
                    return VK_SUCCESS;
                case VK_SUBOPTIMAL_KHR:
                case VK_ERROR_OUT_OF_DATA_KHT:
                    return RecreateSwapchain();
                default:
                    outStream << std::format("[ graphicsBase ] ERROR\nFailed to queue the image for presentation!\nError code: {}\n", int32_t(result));
                    return result;
            }
        }

        result_t PresentImage(VkSeamaphore semaphore_renderingIsOver=VK_NULL_HANDLE)
        {
            VkPresentInfoKHR presentInfo={
              .swapchainCount=1,
              .pSwapchains=&swapchain,
              .pImageIndices=&currentImageIndex
            };
            if(semaphore_renderingIsOver)
            {
                presentInfo.waitSemaphoreCount=1;
                presentInfo.pWaitSemaphores=&semaphore_renderingIsOver;
            }
            return PresentImage(presentInfo);
        }


        void AddCallback_DestroySwapchain(void(*function)()) {
            callbacks_destroySwapchain.push_back(function);
        }

        VkInstance Instance() const {
            return instance;
        }

        const std::vector<const char *> &InstanceLayers() const {
            return instanceLayers;
        }

        const std::vector<const char *> InstanceExtensions() const {
            return instanceExtensions;
        }

        void AddInstanceLayer(const char *layerName) {
            AddLayerOrExtension(instanceLayers, layerName);
        }

        void AddInstanceExtension(const char *extensionName) {
            AddLayerOrExtension(instanceExtensions, extensionName);
        }

        result_t CreateInstance(VkInstanceCreateFlags flags = 0) {
            if constexpr (ENABLE_DEBUG_MESSENGER)
                AddInstanceLayer("VK_LAYER_KHRONOS_validation"),
                        AddInstanceExtension(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
            VkApplicationInfo applicatianInfo = {
                    .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
                    .apiVersion = apiVersion
            };
            VkInstanceCreateInfo instanceCreateInfo = {
                    .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
                    .flags = flags,
                    .pApplicationInfo = &applicatianInfo,
                    .enabledLayerCount = uint32_t(instanceLayers.size()),
                    .ppEnabledLayerNames = instanceLayers.data(),
                    .enabledExtensionCount = uint32_t(instanceExtensions.size()),
                    .ppEnabledExtensionNames = instanceExtensions.data()
            };
            if (VkResult result = vkCreateInstance(&instanceCreateInfo, nullptr, &instance)) {
                std::cout
                        << std::format("[ graphicsBase ] ERROR\nFailed to create a vulkan instance!\nError code: {}\n",
                                       int32_t(result));
                return result;
            }
            std::cout << std::format(
                    "Vulkan API Version: {}.{}.{}\n",
                    VK_VERSION_MAJOR(apiVersion),
                    VK_VERSION_MINOR(apiVersion),
                    VK_VERSION_PATCH(apiVersion));
            if constexpr (ENABLE_DEBUG_MESSENGER)
                CreateDebugMessenger();
            return VK_SUCCESS;
        }


        VkResult CheckInstanceLayers(std::span<const char *> layersToCheck) {
            uint32_t layerCount;
            std::vector<VkLayerProperties> availableLayers;
            if (VkResult result = vkEnumerateInstanceLayerProperties(&layerCount, nullptr)) {
                std::cout << std::format(
                        "[ graphicsBase ] ERROR\n Failed to enumerate instance layers!\n Error code:{}\n",
                        int32_t(result));
                return result;
            }
            if (layerCount) {
                availableLayers.resize(layerCount);
                if (VkResult result = vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data())) {
                    std::cout << std::format(
                            "[ graphicsBase ] ERROR\nFailed to enumerate instance layer properties!\nError code: {}\n",
                            int32_t(result));
                    return result;
                }
                for (auto &i: layersToCheck) {
                    bool found = false;
                    for (auto &j: availableLayers)
                        if (!strcmp(i, j.layerName)) {
                            found = true;
                            break;
                        }
                    if (!found)
                        i = nullptr;
                }
            } else
                for (auto &i: layersToCheck)
                    i = nullptr;
            return VK_SUCCESS;
        }

        void InstanceLayers(const std::vector<const char *> &layerNames) {
            instanceLayers = layerNames;
        }

        VkResult
        CheckInstanceExtensions(std::span<const char *> extensionsToCheck, const char *layerName = nullptr) const {
            uint32_t extensionCount;
            std::vector<VkExtensionProperties> availableExtensions;
            if (VkResult result = vkEnumerateInstanceExtensionProperties(layerName, &extensionCount, nullptr)) {
                layerName ? std::format(
                        "[ graphicsBase ] ERROR\n Failed to enumerate instance extensions for layer {}!\n Error code:{}\n",
                        layerName, int32_t(result)) :
                std::format("[ graphicsBase ] ERROR\n Failed to enumerate instance extensions!\n Error code:{}\n",
                            int32_t(result));
                return result;
            }
            if (extensionCount) {
                availableExtensions.resize(extensionCount);
                if (VkResult result = vkEnumerateInstanceExtensionProperties(layerName, &extensionCount,
                                                                             availableExtensions.data())) {
                    std::cout << std::format(
                            "[ graphicsBase ] ERROR\n Failed to enumerate instance extension properties for layer {}!\n Error code:{}\n",
                            layerName, int32_t(result));
                    return result;
                }
                for (auto &i: extensionsToCheck) {
                    bool found = false;
                    for (auto &j: availableExtensions) {
                        if (!strcmp(i, j.extensionName)) {
                            found = true;
                            break;
                        }
                        if (!found) {
                            i = nullptr;
                        }
                    }
                }
            } else {
                for (auto &i: extensionsToCheck)
                    i = nullptr;
            }
            return VK_SUCCESS;
        }

        void InstanceExtensions(const std::vector<const char *> extensionNames) {
            instanceExtensions = extensionNames;

        }

    private:
        VkDebugUtilsMessengerEXT debugMessenger;

        VkResult CreateDebugMessenger() {
            static PFN_vkDebugUtilsMessengerCallbackEXT DebugUtilsMessengerCallback = [](
                    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                    VkDebugUtilsMessageTypeFlagsEXT messageTypes,
                    const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
                    void *pUserData) -> VkBool32 {
                std::cout << std::format("{}\n\n", pCallbackData->pMessage);
                return VK_FALSE;
            };
            VkDebugUtilsMessengerCreateInfoEXT debugUtilsMessengerCreateInfo = {
                    .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
                    .messageSeverity =
                    VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                    VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
                    .messageType =
                    VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                    VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                    VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
                    .pfnUserCallback = DebugUtilsMessengerCallback
            };
            PFN_vkCreateDebugUtilsMessengerEXT vkCreateDebugUtilsMessenger =
                    reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(instance,
                                                                                               "vkCreateDebugUtilsMessengerEXT"));
            if (vkCreateDebugUtilsMessenger) {
                VkResult result = vkCreateDebugUtilsMessenger(instance, &debugUtilsMessengerCreateInfo, nullptr,
                                                              &debugMessenger);
                if (result)
                    std::cout << std::format(
                            "[ graphicsBase ] ERROR\nFailed to create a debug messenger!\nError code: {}\n",
                            int32_t(result));
                return result;
            }
            std::cout << std::format(
                    "[ graphicsBase ] ERROR\nFailed to get the function pointer of vkCreateDebugUtilsMessengerEXT!\n");
            return VK_RESULT_MAX_ENUM;
        }


    private:
        VkSurfaceKHR surface;
    public:
        VkSurfaceKHR Surface() const {
            return surface;
        }

        void Surface(VkSurfaceKHR surface) {
            if (!this->surface) {
                this->surface = surface;
            }
        }

    private:
        VkPhysicalDevice physicalDevice;
        VkPhysicalDeviceProperties physicalDeviceProperties;
        VkPhysicalDeviceMemoryProperties physicalDeviceMemoryProperties;
        std::vector<VkPhysicalDevice> availablePhysicalDevices;

        VkDevice device;
        uint32_t queueFamilyIndex_graphics = VK_QUEUE_FAMILY_IGNORED;
        uint32_t queueFamilyIndex_presentation = VK_QUEUE_FAMILY_IGNORED;
        uint32_t queueFamilyIndex_compute = VK_QUEUE_FAMILY_IGNORED;
        VkQueue queue_graphics;
        VkQueue queue_presentation;
        VkQueue queue_compute;

        std::vector<const char *> deviceExtensions;

        VkResult
        GetQueueFamilyIndices(VkPhysicalDevice physicalDevice, bool enableGraphicsQueue, bool enableComputeQueue,
                              uint32_t (&queueFamilyIndices)[3]) {
            uint32_t queueFamilyCount = 0;
            vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);
            if (!queueFamilyCount)
                return VK_RESULT_MAX_ENUM;
            std::vector<VkQueueFamilyProperties> queueFamilyPropertieses(queueFamilyCount);
            vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilyPropertieses.data());
            auto &[ig, ip, ic] = queueFamilyIndices;
            ig = ip = ic = VK_QUEUE_FAMILY_IGNORED;
            for (uint32_t i = 0; i < queueFamilyCount; i++) {
                VkBool32 supportGraphics =
                        enableGraphicsQueue && queueFamilyPropertieses[i].queueFlags & VK_QUEUE_GRAPHICS_BIT,
                        supportPresentation = false,
                        supportCompute =
                        enableComputeQueue && queueFamilyPropertieses[i].queueFlags & VK_QUEUE_COMPUTE_BIT;
                if (surface) {
                    if (VkResult result = vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, surface,
                                                                               &supportPresentation)) {
                        std::cout << std::format(
                                "[ graphicsBase ] ERROR\nFailed to determine if the queue family supports presentatio!\nError code: {}\n",
                                int32_t(result));
                        return result;
                    }
                }
                if (supportGraphics && supportCompute) {
                    if (supportPresentation) {
                        ig = ip = ic = i;
                        break;
                    }
                    if (ig != ic || ig == VK_QUEUE_FAMILY_IGNORED)
                        ig = ic = i;
                    if (!surface)
                        break;
                }
                if (supportGraphics && ig == VK_QUEUE_FAMILY_IGNORED)
                    ig = i;
                if (supportPresentation && ip == VK_QUEUE_FAMILY_IGNORED)
                    ip = i;
                if (supportCompute && ic == VK_QUEUE_FAMILY_IGNORED)
                    ic = i;
            }
            if (ig == VK_QUEUE_FAMILY_IGNORED && enableGraphicsQueue || ip == VK_QUEUE_FAMILY_IGNORED && surface ||
                ic == VK_QUEUE_FAMILY_IGNORED && enableComputeQueue)
                return VK_RESULT_MAX_ENUM;
            queueFamilyIndex_graphics = ig;
            queueFamilyIndex_presentation = ip;
            queueFamilyIndex_compute = ic;
            return VK_SUCCESS;
        }

    public:
        VkPhysicalDevice PhysicalDevice() const {
            return physicalDevice;
        }

        const VkPhysicalDeviceProperties &PhysicalDeviceProperties() const {
            return physicalDeviceProperties;
        }

        const VkPhysicalDeviceMemoryProperties &PhysicalDeviceMemoryProperties() const {
            return physicalDeviceMemoryProperties;
        }

        VkPhysicalDevice AvailablePhysicalDevice(uint32_t index) const {
            return availablePhysicalDevices[index];
        }

        uint32_t AvailablePhysicalDeviceCount() const {
            return uint32_t(availablePhysicalDevices.size());
        }

        VkDevice Divice() const {
            return device;
        }

        uint32_t QueueFamilyIndex_Graphics() const {
            return queueFamilyIndex_graphics;
        }

        uint32_t QuueFamilyIndex_Presentation() const {
            return queueFamilyIndex_presentation;
        }

        uint32_t QueueFamilyIndex_Compute() const {
            return queueFamilyIndex_compute;
        }

        VkQueue Queue_Graphics() const {
            return queue_graphics;
        }

        VkQueue Queue_Presentation() const {
            return queue_presentation;
        }

        VkQueue Queue_Compute() const {
            return queue_compute;
        }

        const std::vector<const char *> DeviceExtensions() const {
            return deviceExtensions;
        }

        void AddDeviceExtension(const char *extensionName) {
            AddLayerOrExtension(deviceExtensions, extensionName);
        }

        VkResult GetPhysicalDevices() {
            uint32_t deviceCount;
            if (VkResult result = vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr)) {
                std::cout << std::format(
                        "[ graphicsBase ] ERROR\nFailed to get the count of physical devices!\nError code: {}\n",
                        int32_t(result));
                return result;
            }
            if (!deviceCount)
                std::cout
                        << std::format("[ graphicsBase ] ERROR\nFailed to find any physical device supports vulkan!\n"),
                        abort();
            availablePhysicalDevices.resize(deviceCount);
            VkResult result = vkEnumeratePhysicalDevices(instance, &deviceCount, availablePhysicalDevices.data());
            if (result)
                std::cout << std::format(
                        "[ graphicsBase ] ERROR\nFailed to enumerate physical devices!\nError code: {}\n",
                        int32_t(result));
            return result;
        }

        VkResult
        DeterminePhysicalDevice(uint32_t deviceIndex = 0, bool enableGraphicsQueue, bool enableComputeQueue = true) {
            //TODO
        }

        VkResult CreateDevice(VkDeviceCreateFlags flags = 0) {
            float queuePriority = 1.f;
            VkDeviceQueueCreateInfo queueCreateInfos[3] = {
                    {
                            .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
                            .queueCount = 1,
                            .pQueuePriorities = &queuePriority},
                    {
                            .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
                            .queueCount = 1,
                            .pQueuePriorities = &queuePriority},
                    {
                            .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
                            .queueCount = 1,
                            .pQueuePriorities = &queuePriority}};
            uint32_t queueCreateInfoCount = 0;
            if (queueFamilyIndex_graphics != VK_QUEUE_FAMILY_IGNORED)
                queueCreateInfos[queueCreateInfoCount++].queueFamilyIndex = queueFamilyIndex_graphics;
            if (queueFamilyIndex_presentation != VK_QUEUE_FAMILY_IGNORED &&
                queueFamilyIndex_presentation != queueFamilyIndex_graphics)
                queueCreateInfos[queueCreateInfoCount++].queueFamilyIndex = queueFamilyIndex_presentation;
            if (queueFamilyIndex_compute != VK_QUEUE_FAMILY_IGNORED &&
                queueFamilyIndex_compute != queueFamilyIndex_graphics &&
                queueFamilyIndex_compute != queueFamilyIndex_presentation)
                queueCreateInfos[queueCreateInfoCount++].queueFamilyIndex = queueFamilyIndex_compute;
            VkPhysicalDeviceFeatures physicalDeviceFeatures;
            vkGetPhysicalDeviceFeatures(physicalDevice, &physicalDeviceFeatures);
            VkDeviceCreateInfo deviceCreateInfo = {
                    .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
                    .flags = flags,
                    .queueCreateInfoCount = queueCreateInfoCount,
                    .pQueueCreateInfos = queueCreateInfos,
                    .enabledExtensionCount = uint32_t(deviceExtensions.size()),
                    .ppEnabledExtensionNames = deviceExtensions.data(),
                    .pEnabledFeatures = &physicalDeviceFeatures
            };
            if (VkResult result = vkCreateDevice(physicalDevice, &deviceCreateInfo, nullptr, &device)) {
                std::cout << std::format(
                        "[ graphicsBase ] ERROR\nFailed to create a vulkan logical device!\nError code: {}\n",
                        int32_t(result));
                return result;
            }
            if (queueFamilyIndex_graphics != VK_QUEUE_FAMILY_IGNORED)
                vkGetDeviceQueue(device, queueFamilyIndex_graphics, 0, &queue_graphics);
            if (queueFamilyIndex_presentation != VK_QUEUE_FAMILY_IGNORED)
                vkGetDeviceQueue(device, queueFamilyIndex_presentation, 0, &queue_presentation);
            if (queueFamilyIndex_compute != VK_QUEUE_FAMILY_IGNORED)
                vkGetDeviceQueue(device, queueFamilyIndex_compute, 0, &queue_compute);
            vkGetPhysicalDeviceProperties(physicalDevice, &physicalDeviceProperties);
            vkGetPhysicalDeviceMemoryProperties(physicalDevice, &physicalDeviceMemoryProperties);
            //输出所用的物理设备名称
            std::cout << std::format("Renderer: {}\n", physicalDeviceProperties.deviceName);
            /*待Ch1-4填充*/
            return VK_SUCCESS;
        }

        VkResult
        CheckDeviceExtensions(std::span<const char *> extensionsToCheck, const char *layerName = nullptr) const {
            //TODO
        }

        void DeviceExtensions(const std::vector<const char *> &extensionNames) {
            deviceExtensions = extensionNames;
        }

    private:
        std::vector<VkSurfaceFormatKHR> availableSurfaceFormats;
        VkSwapchainKHR swapchain;
        std::vector<VkImage> swapchainImages;
        std::vector<VkImage> swapchainImageViews;
        VkSwapchainCreateInfoKHR swapchainCreateInfo = {};

        VkResult CreateSwapchain_Internal() {
            if (VkResult result = vkCreateSwapchainKHR(device, &swapchainCreateInfo, nullptr, &swapchain)) {
                std::cout << std::format("[ graphicsBase ] ERROR\nFailed to createw a swapchian!\nError code: {}\n",
                                         int32_t(result));
                return result;
            }
            uint32_t swapchainImageCount;
            if (VkResult result = vkGetSwapchainImagesKHR(device, swapchain, &swapchainImageCount, nullptr)) {
                std::cout << std::format(
                        " [ graphicsBase ] ERROR\nFailed to get the count of swapchain images!\nError code:{}\n",
                        int32_t(result));
                return result;
            }
            swapchainImages.resize(swapchainImageCount);
            if (VkResult result = vkGetSwapchainImagesKHR(device, swapchain, &swapchainImageCount,
                                                          swapchainImages.data())) {
                std::cout << std::format("[ graphicsBase ] ERROR\nFailed to get swapchain images!\n Error code: {}\n",
                                         int32_t(result));
                return result;
            }

            swapchainImageViews.resize(swapchainImageCount);
            VkImageViewCreateInfo imageViewCreateInfo = {
                    .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
                    .viewType = VK_IMAGE_VIEW_TYPE_2D,
                    .format=swapchainCreateInfo.imageFormat,
                    .subresourceRange = {VK_IMAGE_VIEW_TYPE_2D, 0, 1, 0, 1}
            };

            for (size_t i = 0; i < swapchainImageCount; i++) {
                imageViewCreateInfo.image = swapchainImages[i];
                if (VkResult result = vkCreateImageView(device, &imageViewCreateInfo, nullptr,
                                                        &swapchainImageViews[i])) {
                    std::cout << std::format(
                            "[ graphicsBase ] ERROR\nFailed to create image view for swapchain image {}!\nError code: {}\n",
                            i, int32_t(result));
                    return result;
                }
                return VK_SUCCESS;
            }

        }

    public:
        const VkFormat &AvailableSurfaceFormat(uint32_t index) const {
            return availableSurfaceFormats[index].format;
        }

        const VkColorSpaceKHR &AvailableSurfaceColorSpace(uint32_t index) {
            return availableSurfaceFormats[index].colorSpace;
        }

        uint32_t AvailabelSurfaceFormatCount() const {
            return uint32_t(availableSurfaceFormats.size());
        }

        VkSwapchainKHR Swapchain() const {
            return swapchain;
        }

        VkImage SwapchainImage(uint32_t index) const {
            return swapchainImages[index];
        }

        VkImageView SwapchainImageView(uint32_t index) const {

            return swapchainImageViews[index];
        }

        uint32_t SwapchainImageCount() const {
            return uint32_t(swapchainImages.size());
        }

        const VkSwapchainCreateInfoKHR &SwapchainCreateInfo() const {
            return swapchainCreateInfo;
        }

        VkResult GetSurfaceFormats() {
            uint32_t surfaceFormatCount;
            if (VkResult result = vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &surfaceFormatCount,
                                                                       nullptr)) {
                std::cout << std::format(
                        "[ graphicsBase ] ERROR\nFailed to get the count of surface formats!\nError code: {}\n",
                        int32_t(result));
                return result;
            }
            if (!surfaceFormatCount)
                std::cout << std::format("[ graphicsBase ] ERROR\nFailed to find any supported surface format!\n"),
                        abort();
            availableSurfaceFormats.resize(surfaceFormatCount);
            VkResult result = vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &surfaceFormatCount,
                                                                   availableSurfaceFormats.data());
            if (result)
                std::cout << std::format("[ graphicsBase ] ERROR\nFailed to get surface formats!\nError code: {}\n",
                                         int32_t(result));
            return result;
        }

        VkResult SetSurfaceFormat(VkSurfaceFormatKHR surfaceFormat) {
            bool formatIsAvailable = false;
            if (!surfaceFormat.format) {
                //如果格式未指定，只匹配色彩空间，图像格式有啥就用啥
                for (auto &i: availableSurfaceFormats)
                    if (i.colorSpace == surfaceFormat.colorSpace) {
                        swapchainCreateInfo.imageFormat = i.format;
                        swapchainCreateInfo.imageColorSpace = i.colorSpace;
                        formatIsAvailable = true;
                        break;
                    }
            } else
                //否则匹配格式和色彩空间
                for (auto &i: availableSurfaceFormats)
                    if (i.format == surfaceFormat.format &&
                        i.colorSpace == surfaceFormat.colorSpace) {
                        swapchainCreateInfo.imageFormat = i.format;
                        swapchainCreateInfo.imageColorSpace = i.colorSpace;
                        formatIsAvailable = true;
                        break;
                    }
            //如果没有符合的格式，恰好有个语义相符的错误代码
            if (!formatIsAvailable)
                return VK_ERROR_FORMAT_NOT_SUPPORTED;
            //如果交换链已存在，调用RecreateSwapchain()重建交换链
            if (swapchain)
                return RecreateSwapchain();
            return VK_SUCCESS;
        }

        VkResult CreateSwapchain(bool limitFrameRate = true, VkSwapchainCreateFlagsKHR flags = 0) {
            VkSurfaceCapabilitiesKHR surfaceCapabilities = {};
            if (VkResult result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface,
                                                                            &surfaceCapabilities)) {
                std::cout << std::format(
                        "[ graphicsBase ] ERROR\nFailed to get physical device surface capabilities!\nError code: {}\n",
                        int32_t(result));
                return result;
            }
            swapchainCreateInfo.minImageCount = surfaceCapabilities.minImageCount +
                                                (surfaceCapabilities.maxImageCount > surfaceCapabilities.minImageCount);
            swapchainCreateInfo.imageExtent = surfaceCapabilities.currentExtent.width == -1 ? VkExtent2D{
                    glm::clamp(defaultWindowSize.width, surfaceCapabilities.minImageExtent.width,
                               surfaceCapabilities.maxImageExtent.width),
                    glm::clamp(defaultWindowSize.height, surfaceCapabilities.minImageExtent.height,
                               surfaceCapabilities.maxImageExtent.height)
            } : surfaceCapabilities.currentExtent;

            swapchainCreateInfo.preTransform = surfaceCapabilities.currentTransform;
            if (surfaceCapabilities.supportedCompositeAlpha & VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR)
                swapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR;
            else
                for (size_t i = 0; i < 4; i++) {
                    if (surfaceCapabilities.supportedCompositeAlpha & 1 << i)
                        swapchainCreateInfo.compositeAlpha = VkCompositeAlphaFlagBitsKHR(
                                surfaceCapabilities.supportedCompositeAlpha & 1 << i);
                    break;
                }
            swapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
            if (surfaceCapabilities.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_SRC_BIT)
                swapchainCreateInfo.imageUsage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
            if (surfaceCapabilities.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_DST_BIT)
                swapchainCreateInfo.imageUsage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
            else
                std::cout
                        << std::format("[ graphicsBase ] WARNING\nVK_IMAGE_USAGE_TRANSFER_DST_BIT isn't supported!\n");

            if (!availableSurfaceFormats.size()) {
                if (VkResult result = GetSurfaceFormats())
                    return result;
            }
            if (swapchain) {
                for (auto &i: callbakcs_destorySwapchain)
                    i();
                for (auto &i: swapchainImageViews)
                    if (i) {
                        vkDestroyImageView(device, i, nullptr);
                    }
                swapchainImageViews.resize(0);
                vkDestroySwapchainKHR(device, swapchain, nullptr);
                swapchain = VK_NULL_HANDLE;
                swapchainCreateInfo = {};
            }

        }

        VkResult RecreateSwapchain() {
            VkSurfaceCapabilitiesKHR surfaceCapabilities = {};
            if (VkResult result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface,
                                                                            &surfaceCapabilities)) {
                std::cout << std::format(
                        " [ graphicsBase ] ERROR\nFailed to get physical device surface capabilities!\nError code: {}\n",
                        result);
                return result;
            }
            if (surfaceCapabilities.currentExtent.width == 0 || surfaceCapabilities.currentExtent.height == 0) {
                return VK_SUBOPTIMAL_KHR;
            }
            swapchainCreateInfo.imageExtent = surfaceCapabilities.currentExtent;

        }

    private:
        uint32_t apiVersion = VK_API_VERSION_1_0;
    public:
        uint32_t ApiVersion() const {
            return apiVersion;
        }

        VkResult UseLastestApiVersion() {
            if (vkGetInstanceProcAddr(VK_NULL_HANDLE, "vkEnumerateInstanceVersion"))

                return vkEnumerateInstanceVersion(&apiVersion);
            return VK_SUCCESS;
        }


    };

    inline graphicsBase graphicsBase::singleton;
};

#endif //VKBASE_H
