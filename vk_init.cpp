#ifndef VK_INIT_HPP
#define VK_INIT_HPP
#ifdef _WIN32
#include <windows.h>
#endif
#include "vk_init.h"
#include "platform.h"

bool InitVulkan(VkContext* vkcontext, void* window) {
	platform_get_window_size(&vkcontext->screensize.width, &vkcontext->screensize.height);
	VkApplicationInfo appInfo = {};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "Vulkan Triangle";
	appInfo.pEngineName = "Triangle Engine";

	const char* instanceextensions[] = {
		VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
		VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
		VK_KHR_SURFACE_EXTENSION_NAME };


	const char* layers[]{
		"VK_LAYER_KHRONOS_validation"
	};

	/* CREATE INSTANCE INFO */

	VkInstanceCreateInfo instanceInfo = {};
	instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instanceInfo.pApplicationInfo = &appInfo;
	instanceInfo.ppEnabledExtensionNames = instanceextensions;
	instanceInfo.enabledExtensionCount = ArraySize(instanceextensions);
	instanceInfo.ppEnabledLayerNames = layers;
	instanceInfo.enabledLayerCount = ArraySize(layers);
	CHECK_VK((vkCreateInstance(&instanceInfo, 0, &vkcontext->instance)));

	auto vkCreateDebugUtilsMessengerEXT = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(vkcontext->instance, "vkCreateDebugUtilsMessengerEXT");

	if (vkCreateDebugUtilsMessengerEXT)
	{
		VkDebugUtilsMessengerCreateInfoEXT debugInfo = {};
		debugInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		debugInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;
		debugInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
		debugInfo.pfnUserCallback = vk_debug_callback;

		vkCreateDebugUtilsMessengerEXT(vkcontext->instance, &debugInfo, 0, &vkcontext->debugMessenger);
	}
	else
	{
		return false;
	}

	/* CREATE SURFACE */
	VkWin32SurfaceCreateInfoKHR surfaceInfo = {};
	surfaceInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	surfaceInfo.hwnd = (HWND)window;
	surfaceInfo.hinstance = GetModuleHandleA(NULL);
	CHECK_VK(vkCreateWin32SurfaceKHR(vkcontext->instance, &surfaceInfo, 0, &vkcontext->surface));

	// Choose GPU
	vkcontext->graphicsIdx = -1;
	uint32_t gpuCount = 0;
	VkPhysicalDevice gpus[10];

	CHECK_VK(vkEnumeratePhysicalDevices(vkcontext->instance, &gpuCount, 0));
	CHECK_VK(vkEnumeratePhysicalDevices(vkcontext->instance, &gpuCount, gpus));

	for (uint32_t i = 0; i < gpuCount; i++) {
		VkPhysicalDevice gpu = gpus[i];
		uint32_t queueFamilyCount = 0;
		VkQueueFamilyProperties queueProps[10];
		vkGetPhysicalDeviceQueueFamilyProperties(gpu, &queueFamilyCount, 0);
		vkGetPhysicalDeviceQueueFamilyProperties(gpu, &queueFamilyCount, queueProps);

		for (uint32_t j = 0; j < queueFamilyCount; j++) {
			if (queueProps[j].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
				VkBool32 surfaceSupport = VK_FALSE;
				vkGetPhysicalDeviceSurfaceSupportKHR(gpu, j, vkcontext->surface, &surfaceSupport);
				vkcontext->graphicsIdx = j;

				if (surfaceSupport) {
					vkcontext->graphicsIdx = j;
					vkcontext->gpu = gpu;
				}
				break;
			}
		}
	}
	if (vkcontext->graphicsIdx = 0) {
		return false;
	}

	/* LOGICAL DEVICE */
	float queuePriority = 1.0f;

	VkDeviceQueueCreateInfo queueInfo = {};
	queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queueInfo.queueFamilyIndex = vkcontext->graphicsIdx;
	queueInfo.queueCount = 1;
	queueInfo.pQueuePriorities = &queuePriority;

	const char* swapchainExtensions[] = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME,
	};

	VkDeviceCreateInfo deviceInfo = {};
	deviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceInfo.pQueueCreateInfos = &queueInfo;
	deviceInfo.queueCreateInfoCount = 1;
	deviceInfo.ppEnabledExtensionNames = swapchainExtensions;
	deviceInfo.enabledExtensionCount = ArraySize(swapchainExtensions);

	CHECK_VK(vkCreateDevice(vkcontext->gpu, &deviceInfo, NULL, &vkcontext->device));

	vkGetDeviceQueue(vkcontext->device, vkcontext->graphicsIdx, 0, &vkcontext->graphicsQueue);
	/* SWAPCHAIN */
	uint32_t formatCount;
	VkSurfaceFormatKHR surfaceFormats[10];
	CHECK_VK(vkGetPhysicalDeviceSurfaceFormatsKHR(vkcontext->gpu, vkcontext->surface, &formatCount, 0));
	CHECK_VK(vkGetPhysicalDeviceSurfaceFormatsKHR(vkcontext->gpu, vkcontext->surface, &formatCount, surfaceFormats));

	for (uint32_t i = 0; i < formatCount; i++) {
		VkSurfaceFormatKHR format = surfaceFormats[i];
		if (format.format == VK_FORMAT_R8G8B8A8_SRGB && format.colorSpace == VK_COLORSPACE_SRGB_NONLINEAR_KHR) {
			vkcontext->surfaceFormat = format;
			break;
		}
	}
	VkSwapchainCreateInfoKHR swapchainCreateInfo = {};

	VkSurfaceCapabilitiesKHR surfaceCapabilities = {};
	CHECK_VK(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(vkcontext->gpu, vkcontext->surface, &surfaceCapabilities));

	uint32_t imgCount;
	imgCount = surfaceCapabilities.minImageCount + 1;
	imgCount = imgCount > surfaceCapabilities.maxImageCount ? imgCount - 1 : imgCount;

	swapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	swapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	swapchainCreateInfo.presentMode = VK_PRESENT_MODE_FIFO_RELAXED_KHR;
	swapchainCreateInfo.surface = vkcontext->surface;
	swapchainCreateInfo.preTransform = surfaceCapabilities.currentTransform;
	swapchainCreateInfo.imageArrayLayers = 1;
	swapchainCreateInfo.imageExtent = surfaceCapabilities.currentExtent;
	swapchainCreateInfo.minImageCount = imgCount;
	swapchainCreateInfo.imageFormat = vkcontext->surfaceFormat.format;
	CHECK_VK(vkCreateSwapchainKHR(vkcontext->device, &swapchainCreateInfo, 0, &vkcontext->swapchain));

	CHECK_VK(vkGetSwapchainImagesKHR(vkcontext->device, vkcontext->swapchain, &vkcontext->swapchainImageCount, 0));
	CHECK_VK(vkGetSwapchainImagesKHR(vkcontext->device, vkcontext->swapchain, &vkcontext->swapchainImageCount, vkcontext->swapchainimages));

	VkImageViewCreateInfo viewInfo = {};
	viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewInfo.format = vkcontext->surfaceFormat.format;
	viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	viewInfo.subresourceRange.layerCount = 1;
	viewInfo.subresourceRange.levelCount = 1;
	for (uint32_t i = 0; i < vkcontext->swapchainImageCount; i++) {
		viewInfo.image = vkcontext->swapchainimages[i];
		CHECK_VK(vkCreateImageView(vkcontext->device, &viewInfo, 0, &vkcontext->swapchainImageViews[i]));
	}
	/* RENDER PASS */

	VkAttachmentDescription attachment = {};
	attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachment.samples = VK_SAMPLE_COUNT_1_BIT;
	attachment.format = vkcontext->surfaceFormat.format;

	// Subpass

	VkAttachmentReference colorAttachmentRef = {};
	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpassDescription = {};
	subpassDescription.colorAttachmentCount = 1;
	subpassDescription.pColorAttachments = &colorAttachmentRef;

	VkAttachmentDescription attachments[] = {
		attachment
	};

	VkRenderPassCreateInfo renderpassInfo = {};
	renderpassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderpassInfo.pAttachments = attachments;
	renderpassInfo.attachmentCount = ArraySize(attachments);
	renderpassInfo.subpassCount = 1;
	renderpassInfo.pSubpasses = &subpassDescription;
	CHECK_VK(vkCreateRenderPass(vkcontext->device, &renderpassInfo, 0, &vkcontext->renderpass));

	/* FRAMEBUFFERS */

	VkFramebufferCreateInfo framebufferInfo = {};
	framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	framebufferInfo.width = vkcontext->screensize.width;
	framebufferInfo.height = vkcontext->screensize.height;
	framebufferInfo.renderPass = vkcontext->renderpass;
	framebufferInfo.layers = 1;
	framebufferInfo.attachmentCount = 1;

	for (uint32_t i = 0; i < vkcontext->swapchainImageCount; i++) {
		framebufferInfo.pAttachments = &vkcontext->swapchainImageViews[i];
		vkCreateFramebuffer(vkcontext->device, &framebufferInfo, 0, &vkcontext->framebuffers[i]);
	}

	/* PIPELINE LAYOUT */
	VkPipelineLayoutCreateInfo layoutInfo = {};
	layoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	CHECK_VK(vkCreatePipelineLayout(vkcontext->device, &layoutInfo, 0, &vkcontext->pipelineLayout));

	/* PIPELINE */

	VkShaderModule vertexShader, fragmentShader;

	uint32_t sizeInBytes;
	char* code = platform_read_file("shaders/shader.vert.spv", &sizeInBytes);

	VkShaderModuleCreateInfo shaderInfo = { };
	shaderInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	shaderInfo.pCode = (uint32_t*)code;
	shaderInfo.codeSize = sizeInBytes;
	CHECK_VK(vkCreateShaderModule(vkcontext->device, &shaderInfo, 0, &vertexShader));
	delete code;

	code = platform_read_file("shaders/shader.frag.spv", &sizeInBytes);
	shaderInfo.pCode = (uint32_t*)code;
	shaderInfo.codeSize = sizeInBytes;
	CHECK_VK(vkCreateShaderModule(vkcontext->device, &shaderInfo, 0, &fragmentShader));
	delete code;

	VkPipelineShaderStageCreateInfo vertexStage = {};
	vertexStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertexStage.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vertexStage.pName = "main";
	vertexStage.module = vertexShader;

	VkPipelineShaderStageCreateInfo fragmentStage = {};
	fragmentStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragmentStage.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragmentStage.pName = "main";
	fragmentStage.module = fragmentShader;

	VkPipelineShaderStageCreateInfo shaderStages[] = {
		vertexStage,
		fragmentStage
	};

	VkPipelineVertexInputStateCreateInfo vertexInputState = {};
	vertexInputState.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

	VkPipelineColorBlendAttachmentState colorAttachment = {};
	colorAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorAttachment.blendEnable = VK_FALSE;

	VkPipelineColorBlendStateCreateInfo colorBlendState = {};
	colorBlendState.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlendState.pAttachments = &colorAttachment;
	colorBlendState.attachmentCount = 1;

	/*
	VkPipelineCache pipelineCache = {};
	VkPipelineCacheCreateInfo pipelineCacheInfo = {};
	pipelineCacheInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
	pipelineCacheInfo.flags = ;
	pipelineCacheInfo.initialDataSize = ;
	*/

	VkPipelineRasterizationStateCreateInfo rasterizationState = {};
	rasterizationState.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizationState.frontFace = VK_FRONT_FACE_CLOCKWISE;
	rasterizationState.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizationState.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizationState.lineWidth = 1.0f;

	VkPipelineMultisampleStateCreateInfo multisampleState = {};
	multisampleState.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampleState.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

	VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
	inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

	VkRect2D scissor = {};
	VkViewport viewport = {};

	VkPipelineViewportStateCreateInfo viewportState = { };
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.scissorCount = 1;
	viewportState.pScissors = &scissor;
	viewportState.viewportCount = 1;
	viewportState.pViewports = &viewport;

	VkDynamicState dynamicStates[] = {
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_SCISSOR
	};

	VkPipelineDynamicStateCreateInfo dynamicState = {};
	dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicState.pDynamicStates = dynamicStates;
	dynamicState.dynamicStateCount = ArraySize(dynamicStates);

	VkGraphicsPipelineCreateInfo pipelineInfo = {};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.layout = vkcontext->pipelineLayout;
	pipelineInfo.renderPass = vkcontext->renderpass;
	pipelineInfo.pVertexInputState = &vertexInputState;
	pipelineInfo.pColorBlendState = &colorBlendState;
	pipelineInfo.pStages = shaderStages;
	pipelineInfo.stageCount = ArraySize(shaderStages);
	pipelineInfo.pRasterizationState = &rasterizationState;
	pipelineInfo.pViewportState = &viewportState;
	pipelineInfo.pDynamicState = &dynamicState;
	pipelineInfo.pMultisampleState = &multisampleState;
	pipelineInfo.pInputAssemblyState = &inputAssembly;
	vkCreateGraphicsPipelines(vkcontext->device, 0, 1, &pipelineInfo, 0 , &vkcontext->pipeline);

	/* COMMAND POOL */
	VkCommandPoolCreateInfo poolInfo = {};
	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.queueFamilyIndex = vkcontext->graphicsIdx;
	vkCreateCommandPool(vkcontext->device, &poolInfo, 0, &vkcontext->commandPool);

	/* SYNC OBJECTS */
	VkSemaphoreCreateInfo semaphoreInfo = {};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	CHECK_VK(vkCreateSemaphore(vkcontext->device, &semaphoreInfo, 0, &vkcontext->acquireSemaphore));
	CHECK_VK(vkCreateSemaphore(vkcontext->device, &semaphoreInfo, 0, &vkcontext->submitSemaphore));

	return true;
}

#endif // VK_INIT_HPP