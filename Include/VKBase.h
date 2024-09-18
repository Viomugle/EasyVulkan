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
                    .sType=VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
                    .flags=flags,
                    .pApplicationInfo=&applicatianInfo,
                    .enabledLayerCount=uint32_t(instanceLayers.size()),
                    .ppEnabledLayerNames=instanceLayers.data(),
                    .enabledExtensionCount=uint32_t(instanceExtensions.size()),
                    .ppEnabledExtensionNames=instanceExtensions.data()
            };

            if(VkResult result=vkCreateInstance(&instanceCreateInfo,nullptr,&instance))
            {
                std::cout<<std::format("[ graphicsBase ] ERROR\n Failed to create a vulkan instance!\n Error code:{}\n",int32_t(result));
                return result;
            }

            std::cout<<std::format(
                    "Vulkan API Version: {}.{}.{}\n",
                    VK_VERSION_MAJOR(apiVersion),
                    VK_VERSION_MINOR(apiVersion),
                    VK_VERSION_PATCH(apiVersion)
                    );
#ifndef NDEBUG
            CreateDebugMessenger();
#endif
            return VK_SUCCESS;
        }
        VkResult CheckInstanceLayers(std::span<const char*> layersToCheck)
        {
            uint32_t layerCount;
            std::vector<VkLayerProperties >availableLayers;
            if(VkResult result= vkEnumerateInstanceLayerProperties(&layerCount, nullptr))
            {
                std::cout<<std::format("[ graphicsBase ] ERROR\n Failed to enumerate instance layers!\n Error code:{}\n",int32_t(result));
                return result;
            }
            if (layerCount) {
                availableLayers.resize(layerCount);
                if (VkResult result = vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data())) {
                    std::cout << std::format("[ graphicsBase ] ERROR\nFailed to enumerate instance layer properties!\nError code: {}\n", int32_t(result));
                    return result;
                }
                for (auto& i : layersToCheck) {
                    bool found = false;
                    for (auto& j : availableLayers)
                        if (!strcmp(i, j.layerName)) {
                            found = true;
                            break;
                        }
                    if (!found)
                        i = nullptr;
                }
            }
            else
                for (auto& i : layersToCheck)
                    i = nullptr;
            return VK_SUCCESS;
        }
        void InstanceLayers(const std::vector<const char*>& layerNames)
        {
            instanceLayers=layerNames;
        }
        VkResult CheckInstanceExtensions(std::span<const char*> extensionsToCheck,const char* layerName=nullptr)const
        {
            uint32_t extensionCount;
            std::vector<VkExtensionProperties> availableExtensions;
            if(VkResult result=vkEnumerateInstanceExtensionProperties(layerNmae,&extensionCount,nullptr))
            {
                layerName?std::format("[ graphicsBase ] ERROR\n Failed to enumerate instance extensions for layer {}!\n Error code:{}\n",layerName,int32_t(result)):
                std::format("[ graphicsBase ] ERROR\n Failed to enumerate instance extensions!\n Error code:{}\n",int32_t(result));
                return result;
            }
            if(extensionCount)
            {
                availableExtensions.resize(extensionCount);
                if(VkResult result = vkEnumerateInstanceExtensionProperties(layerName, &extensionCount, availableExtensions.data()))
                {
                    std::cout<<std::format("[ graphicsBase ] ERROR\n Failed to enumerate instance extension properties for layer {}!\n Error code:{}\n",layerName,int32_t(result));
                    return result;
                }
                for(auto& i:extensionsToCheck)
                {
                    bool found=false;
                    for(auto& j:availableExtensions)
                    {
                        if(!strcmp(i,j.extensionName))
                        {
                            found=true;
                            break;
                        }
                        if(!found)
                        {
                            i=nullptr;
                        }
                    }
                }
            }
            else
            {
                for(auto& i:extensionsToCheck)
                    i=nullptr;
            }
            return VK_SUCCESS;
        }
        void InstanceExtensions(const std::vector<const char*> extensionNames)
        {
            instanceExtensions=extensionNames;

        }

    private:
        VkDebugUtilsMessengerEXT debugMessenger;

        VkResult CreateDebugMessenger() {
            static PFN_vkDebugUtilsMessengerCallbackEXT DebugUtilsMessengerCallback = [](
                    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                    VkDebugUtilsMessageTypeFlagsEXT messageTypes,
                    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                    void* pUserData)->VkBool32 {
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
                    reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT"));
            if (vkCreateDebugUtilsMessenger) {
                VkResult result = vkCreateDebugUtilsMessenger(instance, &debugUtilsMessengerCreateInfo, nullptr, &debugMessenger);
                if (result)
                    std::cout << std::format("[ graphicsBase ] ERROR\nFailed to create a debug messenger!\nError code: {}\n", int32_t(result));
                return result;
            }
            std::cout << std::format("[ graphicsBase ] ERROR\nFailed to get the function pointer of vkCreateDebugUtilsMessengerEXT!\n");
            return VK_RESULT_MAX_ENUM;
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
        VkResult GetQueueFamilyIndices(VkPhysicalDevice physicalDevice, bool enableGraphicsQueue, bool enableComputeQueue, uint32_t (&queueFamilyIndices)[3]) {
            uint32_t queueFamilyCount = 0;
            vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);
            if (!queueFamilyCount)
                return VK_RESULT_MAX_ENUM;
            std::vector<VkQueueFamilyProperties> queueFamilyPropertieses(queueFamilyCount);
            vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilyPropertieses.data());
            auto& [ig,ip,ic]=queueFamilyIndices;
            ig=ip=ic=VK_QUEUE_FAMILY_IGNORED;
            for(uint32_t i=0;i<queueFamilyCount;i++)
            {
                VkBool32 supportGraphics=enableGraphicsQueue &&queueFamilyPropertieses[i].queueFlags&VK_QUEUE_GRAPHICS_BIT,
                supportPresentation=false,
                supportCompute=enableComputeQueue&&queueFamilyPropertieses[i].queueFlags&VK_QUEUE_COMPUTE_BIT;
                if(surface)
                {
                    if(VkResult result= vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice,i,surface,&supportPresentation))
                    {
                        std::cout<<std::format("[ graphicsBase ] ERROR\nFailed to determine if the queue family supports presentatio!\nError code: {}\n",int32_t(result));
                        return result;
                    }
                }
                if(supportGraphics&&supportCompute)
                {
                    if(supportPresentation)
                    {
                        ig=ip=ic=i;
                        break;
                    }
                    if(ig!=ic||ig==VK_QUEUE_FAMILY_IGNORED)
                        ig=ic=i;
                    if(!surface)
                        break;
                }
                if(supportGraphics&&ig==VK_QUEUE_FAMILY_IGNORED)
                    ig=i;
                if(supportPresentation&&ip==VK_QUEUE_FAMILY_IGNORED)
                    ip=i;
                if(supportCompute&&ic==VK_QUEUE_FAMILY_IGNORED)
                    ic=i;
            }
            if(ig==VK_QUEUE_FAMILY_IGNORED&&enableGraphicsQueue||ip==VK_QUEUE_FAMILY_IGNORED&&surface||ic==VK_QUEUE_FAMILY_IGNORED&&enableComputeQueue)
                return VK_RESULT_MAX_ENUM;
            queueFamilyIndex_graphics=ig;
            queueFamilyIndex_presentation=ip;
            queueFamilyIndex_compute=ic;
            return VK_SUCCESS;
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

        VkResult GetPhysicalDevices() {
            uint32_t deviceCount;
            if (VkResult result = vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr)) {
                std::cout << std::format("[ graphicsBase ] ERROR\nFailed to get the count of physical devices!\nError code: {}\n", int32_t(result));
                return result;
            }
            if (!deviceCount)
                std::cout << std::format("[ graphicsBase ] ERROR\nFailed to find any physical device supports vulkan!\n"),
                        abort();
            availablePhysicalDevices.resize(deviceCount);
            VkResult result = vkEnumeratePhysicalDevices(instance, &deviceCount, availablePhysicalDevices.data());
            if (result)
                std::cout << std::format("[ graphicsBase ] ERROR\nFailed to enumerate physical devices!\nError code: {}\n", int32_t(result));
            return result;
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
