#pragma once

#include <vulkan/vulkan.h>
#include <vulkan/vulkan_win32.h>

struct VkContext {
	VkInstance instance;
	VkSurfaceKHR surface;
	VkSurfaceFormatKHR surfaceFormat;
	VkPhysicalDevice gpu;
	VkDevice device;

	VkSwapchainKHR swapchain;
	VkImage swapchainimages[5];
	uint32_t swapchainImageCount;
	VkImageView swapchainImageViews[5];

	VkQueue graphicsQueue;
	VkDebugUtilsMessengerEXT debugMessenger;
	VkCommandPool commandPool;

	VkSemaphore submitSemaphore;
	VkSemaphore acquireSemaphore;

	VkRenderPass renderpass;
	VkExtent2D screensize;
	VkFramebuffer framebuffers[5];

	VkPipeline pipeline;
	VkPipelineLayout pipelineLayout;
	int graphicsIdx;
};