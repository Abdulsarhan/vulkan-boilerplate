#ifndef VK_RENDERER_H
#define VK_RENDERER_H
#include "vk_init.h"
#include "vk_context.h"

bool Renderer(VkContext* vkcontext) {
	uint32_t imgIdx;
	CHECK_VK(vkAcquireNextImageKHR(vkcontext->device, vkcontext->swapchain, 0, vkcontext->acquireSemaphore, 0, &imgIdx));

	/* COMMAND BUFFER */
	VkCommandBuffer cmd;
	VkCommandBufferAllocateInfo allocateInfo = {};
	allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocateInfo.commandBufferCount = 1;
	allocateInfo.commandPool = vkcontext->commandPool;
	CHECK_VK(vkAllocateCommandBuffers(vkcontext->device, &allocateInfo, &cmd));

	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	/* RENDERING COMMANDS */
	CHECK_VK(vkBeginCommandBuffer(cmd, &beginInfo));
	VkClearColorValue clearColor = { 1, 1, 0, 1 };
	VkClearValue clearValue = {};
	clearValue.color = clearColor;
	VkImageSubresourceRange range = {};
	range.layerCount = 1;
	range.levelCount = 1;
	range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	vkCmdClearColorImage(cmd, vkcontext->swapchainimages[imgIdx], VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &clearColor, 1, &range);

	VkRenderPassBeginInfo renderpassBeginInfo = {};
	renderpassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderpassBeginInfo.renderPass = vkcontext->renderpass;
	renderpassBeginInfo.renderArea.extent = vkcontext->screensize;
	renderpassBeginInfo.framebuffer = vkcontext->framebuffers[imgIdx];
	renderpassBeginInfo.clearValueCount = 1;
	renderpassBeginInfo.pClearValues = &clearValue;

	vkCmdBeginRenderPass(cmd, &renderpassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

	VkRect2D scissor = {};
	scissor.extent = vkcontext->screensize;

	VkViewport viewport = {};
	viewport.height = vkcontext->screensize.height;
	viewport.width = vkcontext->screensize.width;
	viewport.maxDepth = 1.0f;

	vkCmdSetScissor(cmd, 0, 1, &scissor);
	vkCmdSetViewport(cmd, 0, 1, &viewport);

	vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, vkcontext->pipeline);
	vkCmdDraw(cmd, 3, 1, 0, 0);
	vkCmdEndRenderPass(cmd);

	CHECK_VK(vkEndCommandBuffer(cmd));

	/* SUBMIT INFO */
	VkSubmitInfo submitInfo = {};
	VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &cmd;
	submitInfo.pWaitDstStageMask = &waitStage;
	submitInfo.pSignalSemaphores = &vkcontext->submitSemaphore;
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = &vkcontext->acquireSemaphore;
	submitInfo.waitSemaphoreCount = 1;
	CHECK_VK(vkQueueSubmit(vkcontext->graphicsQueue, 1, &submitInfo, 0));

	/* PRESENT INFO */
	VkPresentInfoKHR presentInfo = {};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.pSwapchains = &vkcontext->swapchain;
	presentInfo.swapchainCount = 1;
	presentInfo.pImageIndices = &imgIdx;
	presentInfo.pWaitSemaphores = &vkcontext->submitSemaphore;
	presentInfo.waitSemaphoreCount = 1;
	CHECK_VK(vkQueuePresentKHR(vkcontext->graphicsQueue, &presentInfo));

	CHECK_VK(vkDeviceWaitIdle(vkcontext->device));
	vkFreeCommandBuffers(vkcontext->device, vkcontext->commandPool, 1, &cmd);

	return true;
}

#endif //VK_RENDERER_H