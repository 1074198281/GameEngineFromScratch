#pragma once
#include "CocoaMetalApplication.h"
#include "vulkan/vulkan.hpp"

namespace My {
class CocoaVulkanApplication : public CocoaMetalApplication {
   public:
    using CocoaMetalApplication::CocoaMetalApplication;
    VkResult CreateWindowSurface(vk::Instance instance, VkSurfaceKHR& surface);
};
}  // namespace My
