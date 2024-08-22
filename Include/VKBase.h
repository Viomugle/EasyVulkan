//
// Created by 24510 on 2024/8/21.
//

#ifndef VKBASE_H
#define VKBASE_H

#include "EasyVulkanStart.h"

namespace vulkan
{
    class graphicsBase{
        static graphicsBase singleton;
        graphicsBase()=default;
        graphicsBase(const graphicsBase&&)=delete;
        ~graphicsBase(){}
    public:
        static graphicsBase& Base()
        {
            return singleton;
        }
    private:
        VkInstance instance;
        std::vector<const char*> instanceLayers;
        std::vector<const char*> instanceExtensions;
        static void AddLayerOrExtension(std::vector<const char*>& container,const char* name)
        {
            for(auto& i:container)
            {
                if(!strcmp(name,i))
                    return ;
            }
            container.push_back(name);
        }

    public:
        VkInstance Instance()const{
            return instance;
        }
        const std::vector<const char*>& InstanceLayers() const{
            return instanceLayers;
        }
        const std::vector<const char*> InstanceExtensions()const{
            return instanceExtensions;
        }

        void AddInstanceLayer(const char* layerName)
        {
            AddLayerOrExtension(instanceLayers,layerName);
        }

        void AddInstanceExtension(const char* extensionName)
        {
            AddLayerOrExtension(instanceExtensions,extensionName);
        }

        VkResult CreateInstance(VkInstanceCreateFlags flags=0)
        {
#ifndef NDEBUG

            AddInstanceLayer("VK_LAYER_KHRONOS_validation");
            AddInstanceExtension(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

#endif

            VkApplicationInfo applicatianInfo={
                    .sType=VK_STRUCTURE_TYPE_APPLICATION_INFO,
                    .apiVersion=apiVersion
            };
            VkInstanceCreateInfo instanceCreateInfo={
                    .sType=VK_STRUTURE_TYPE_INSTANCE_CREATE_INFO,
                    .flags=flags,
                    .pApplicationInfo=&applicatianInfo,
                    .enabledLayerCount=uint32_t(instanceLayers.size()),
                    .ppEnableLayerNames=instanceLayers.data(),
                    .enabledExtensionCount=uint32_t(instaceExtensions.size()),
                    .ppEnableExtensionNames=instanceExtensions.data()
            };

            if(VkResult result=vkR)

        }
        VkResult CheckInstanceLayers(std::span<const char*> layersToCheck)
        {
            //TODO
        }
        void InstanceLayers(const std::vector<const char*>& layerNames)
        {
            instanceLayers=layerNames;
        }
        VkResult CheckInstanceExtensions(std::span<const char*> extensionsToCheck,const char* layerName=nullptr)const
        {
            //TODO
        }
        void InstanceExtensions(const std::vector<const char*> extensionNames)
        {
            instanceExtensions=extensionNames;

        }

    private:
        VkDebugUtilsMessengerEXT debugMessenger;
        VkResult CreateDebugMessenger()
        {
            //TODO
        }

    private:
        VkSurfaceKHR surface;
    public:
        VkSurfaceKHR Surface() const{
            return surface;
        }
        void Surface(VkSurfaceKHR surface)
        {
            if(!this->surface)
            {
                this->surface=surface;
            }
        }

    private:
        VkPhysicalDevice physicalDevice;
        VkPhysicalDeviceProperties physicalDeviceProperties;
        VkPhysicalDeviceMemoryProperties  physicalDeviceMemoryProperties;
        std::vector<VkPhysicalDevice> availablePhysicalDevices;

        VkDevice device;
        uint32_t queueFamilyIndex_graphics=VK_QUEUE_FAMILY_IGNORED;
        uint32_t queueFamilyIndex_presentation=VK_QUEUE_FAMILY_IGNORED;
        uint32_t queueFamilyIndex_compute=VK_QUEUE_FAMILY_IGNORED;
        VkQueue queue_graphics;
        VkQueue queue_presentation;
        VkQueue queue_compute;

        std::vector<const char*> deviceExtensions;
        VkResult GetQueueFamilyIndices(VkPhysicalDevice physicalDevice,bool enableGraphicsQueue,bool enableComputeQueue,uint32_t (&ueueFamilyIndices)[3])
        {
            //TODO
        }
    public:
        VkPhysicalDevice PhysicalDevice()const{
            return physicalDevice;
        }
        const VkPhysicalDeviceProperties& PhysicalDeviceProperties() const
        {
            return physicalDeviceProperties;
        }

        const VkPhysicalDeviceMemoryProperties & PhysicalDeviceMemoryProperties()const
        {
            return physicalDeviceMemoryProperties;
        }

        VkPhysicalDevice AvailablePhysicalDevice(uint32_t index) const
        {
            return availablePhysicalDevices[index];
        }

        uint32_t AvailablePhysicalDeviceCount()const
        {
            return uint32_t (availablePhysicalDevices.size());
        }

        VkDevice Divice() const
        {
            return device;
        }

        uint32_t QueueFamilyIndex_Graphics()const{
            return queueFamilyIndex_graphics;
        }
        uint32_t QuueFamilyIndex_Presentation()const{
            return queueFamilyIndex_presentation;
        }
        uint32_t QueueFamilyIndex_Compute()const{
            return queueFamilyIndex_compute;
        }
        VkQueue Queue_Graphics()const{
            return queue_graphics;
        }
        VkQueue Queue_Presentation()const{
            return queue_presentation;
        }
        VkQueue Queue_Compute()const{
            return queue_compute;
        }

        const std::vector<const char*> DeviceExtensions()const{
            return deviceExtensions;
        }

        void AddDeviceExtension(const char* extensionName)
        {
            AddLayerOrExtension(deviceExtensions,extensionName);
        }

        VkResult GetPhysicalDevice()
        {
            //TODO
        }
        VkResult DeterminePhysicalDevice(uint32_t deviceIndex=0,bool enableGraphicsQueue,bool enableComputeQueue=true)
        {
            //TODO
        }

        VkResult CreateDevice(VkDeviceCreateFlags flags=0)
        {
            //TODO
        }

        VkResult CheckDeviceExtensions(std::span<const char*> extensionsToCheck,const char* layerName=nullptr)const
        {
            //TODO
        }

        void DeviceExtensions(const std::vector<const char*>& extensionNames)
        {
            deviceExtensions=extensionNames;
        }

    private:
        std::vector<VkSurfaceFormatKHR> availableSurfaceFormats;
        VkSwapchainKHR swapchain;
        std::vector<VkImage> swapchainImages;
        std::vector<VkImage> swapchainImageViews;
        VkSwapchainCreateInfoKHR swapchainCreateInfo={};
        VkResult CreateSwapchain_Internal()
        {
            //TODO
        }
    public:
        const VkFormat& AvailableSurfaceFormat(uint32_t index)const
        {
            return availableSurfaceFormats[index].format;
        }
        const VkColorSpaceKHR& AvailableSurfaceColorSpace(uint32_t index)
        {
            return availableSurfaceFormats[index].colorSpace;
        }
        uint32_t AvailabelSurfaceFormatCount()const
        {
            return uint32_t(availableSurfaceFormats.size());
        }
        VkSwapchainKHR Swapchain()const
        {
            return swapchain;
        }

        VkImage SwapchainImage(uint32_t index)const
        {
            return swapchainImages[index];
        }
        VkImageView SwapchainImageView(uint32_t index) const
        {
        return swapchainImagesViews[index];
        }
        uint32_t SwapchainImageCount() const
        {
            return uint32_t(swapchainImages.size());
        }

        const VkSwapchainCreateInfoKHR& SwapchainCreateInfo()const
        {
            return swapchainCreateInfo;
        }
        VkResult GetSurfaceFormats()
        {
            //TODO
        }

        VkResult CreateSwapchain(bool limitFrameRate=true,VkSwapchainCreateFlagsKHR flags=0)
        {
            //TODO
        }

        VkResult RecreateSwapchain()
        {
            //TODO
        }

    private:
        uint32_t apiVersion=VK_API_VERSION_1_0;
    public:
        uint32_t ApiVersion() const
        {
            return apiVersion;
        }
        VkResult UseLastestApiVersion()
        {
            if(vkGetInstanceProcAddr(VK_NULL_HANLDE,"vkEnumerateInstanceVersion"))
                return vkEnumerteInstanceVersion(&apiVersion);
            return VK_SUCCESS;
        }



    };
    inline graphicsBase graphicsBase::singleton;
};

#endif //VKBASE_H
