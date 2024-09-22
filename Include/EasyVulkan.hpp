//
// Created by 24510 on 2024/9/22.
//

#ifndef MYVULKAN_EASYVULKAN_HPP
#define MYVULKAN_EASYVULKAN_HPP

#include <VKBase.h>
#include <tools.hpp>

using namespace vulkan;
const VkExtent2D& windowSize=graphicsBase::Base().SwapchainCreateInfo().imageExtent;
namespace easyVulkan{
    using namespace vulkan;
    struct renderPassWithFramebuffers{
        renderPass renderPass;
        std::vector<framebuffer> framebuffers;
    };

    const auto& CreateRpwf_Screen()
    {
        static renderPassWithFramebuffers  rpwf;
        //todo
        return rpwf;
    }

}

#endif //MYVULKAN_EASYVULKAN_HPP
