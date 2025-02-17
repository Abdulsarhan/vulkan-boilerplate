#ifndef VK_INIT_H
#define VK_INIT_H

#include <vulkan/vulkan.h>
#include <vulkan/vulkan_win32.h>
#include <iostream>
#include "vk_context.h"

static VKAPI_ATTR VkBool32 VKAPI_CALL vk_debug_callback(
	VkDebugUtilsMessageSeverityFlagBitsEXT msgSeverity,
	VkDebugUtilsMessageTypeFlagsEXT msgFlags,
	const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
	void* pUserData)
{
	std::cout << "Validation Error: " << pCallbackData->pMessage << "\n" << std::endl;
	return false;
}

#define ArraySize(arr) sizeof((arr)) / sizeof((arr[0]))

#define CHECK_VK(result)                                      \
    if (result != VK_SUCCESS)                                 \
    {                                                         \
        std::cout << "Vulkan Error: " << result << std::endl; \
        __debugbreak();                                       \
        return false;                                         \
    }

bool InitVulkan(VkContext* vkcontext, void* window);

#endif // VK_INIT_H
