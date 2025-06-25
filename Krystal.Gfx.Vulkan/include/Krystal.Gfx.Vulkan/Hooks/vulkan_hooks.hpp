#pragma once

#include "Krystal.Core/Core.hpp"

#define VK_NO_PROTOTYPES
#include <vulkan/vulkan.h>

namespace Krys::Gfx::Vulkan
{
  namespace Hooks
  {
    NO_DISCARD bool HookCreateInstanceFunction() noexcept;

    NO_DISCARD bool HookEntryFunctions() noexcept;

    NO_DISCARD bool HookInstanceFunctions(VkInstance instance) noexcept;

    NO_DISCARD bool HookPlatformInstanceFunctions(VkInstance instance) noexcept;

    NO_DISCARD bool HookDeviceFunctions(VkDevice device) noexcept;

    NO_DISCARD bool HookDebugMessengerFunctions(VkInstance instance) noexcept;
  }

  NO_DISCARD VkSurfaceKHR CreateSurface(VkInstance instance, NativeHandle windowHandle);

#define KRYS_DECLARE_VK_FUNC_PTR(name) inline PFN_##name name = nullptr;

  KRYS_DECLARE_VK_FUNC_PTR(vkGetInstanceProcAddr)

#pragma region Instance Functions

  KRYS_DECLARE_VK_FUNC_PTR(vkCreateInstance)
  KRYS_DECLARE_VK_FUNC_PTR(vkDestroyInstance)
  KRYS_DECLARE_VK_FUNC_PTR(vkCreateDevice)
  KRYS_DECLARE_VK_FUNC_PTR(vkGetDeviceProcAddr)
  KRYS_DECLARE_VK_FUNC_PTR(vkEnumeratePhysicalDevices)
  KRYS_DECLARE_VK_FUNC_PTR(vkGetPhysicalDeviceProperties)
  KRYS_DECLARE_VK_FUNC_PTR(vkGetPhysicalDeviceFeatures)
  KRYS_DECLARE_VK_FUNC_PTR(vkGetPhysicalDeviceFormatProperties)
  KRYS_DECLARE_VK_FUNC_PTR(vkGetPhysicalDeviceImageFormatProperties)
  KRYS_DECLARE_VK_FUNC_PTR(vkGetPhysicalDeviceQueueFamilyProperties)
  KRYS_DECLARE_VK_FUNC_PTR(vkGetPhysicalDeviceMemoryProperties)
  KRYS_DECLARE_VK_FUNC_PTR(vkEnumerateInstanceExtensionProperties)
  KRYS_DECLARE_VK_FUNC_PTR(vkEnumerateInstanceLayerProperties)
  KRYS_DECLARE_VK_FUNC_PTR(vkEnumeratePhysicalDeviceGroups)
  KRYS_DECLARE_VK_FUNC_PTR(vkEnumerateDeviceExtensionProperties)
  KRYS_DECLARE_VK_FUNC_PTR(vkGetPhysicalDeviceExternalBufferProperties)
  KRYS_DECLARE_VK_FUNC_PTR(vkGetPhysicalDeviceExternalFenceProperties)
  KRYS_DECLARE_VK_FUNC_PTR(vkGetPhysicalDeviceExternalSemaphoreProperties)

#pragma region Debug

  KRYS_DECLARE_VK_FUNC_PTR(vkCreateDebugUtilsMessengerEXT)
  KRYS_DECLARE_VK_FUNC_PTR(vkDestroyDebugUtilsMessengerEXT)
  KRYS_DECLARE_VK_FUNC_PTR(vkSubmitDebugUtilsMessageEXT)

#pragma endregion

#pragma region Surface

  KRYS_DECLARE_VK_FUNC_PTR(vkDestroySurfaceKHR)
  KRYS_DECLARE_VK_FUNC_PTR(vkGetPhysicalDeviceSurfaceSupportKHR)
  KRYS_DECLARE_VK_FUNC_PTR(vkGetPhysicalDeviceSurfaceCapabilitiesKHR)
  KRYS_DECLARE_VK_FUNC_PTR(vkGetPhysicalDeviceSurfaceFormatsKHR)
  KRYS_DECLARE_VK_FUNC_PTR(vkGetPhysicalDeviceSurfacePresentModesKHR)

#pragma endregion

#pragma endregion

#pragma region Device Functions

  KRYS_DECLARE_VK_FUNC_PTR(vkDestroyDevice)
  KRYS_DECLARE_VK_FUNC_PTR(vkGetDeviceQueue)

  KRYS_DECLARE_VK_FUNC_PTR(vkCreateShaderModule)
  KRYS_DECLARE_VK_FUNC_PTR(vkDestroyShaderModule)

  KRYS_DECLARE_VK_FUNC_PTR(vkCreatePipelineLayout)
  KRYS_DECLARE_VK_FUNC_PTR(vkDestroyPipelineLayout)

  KRYS_DECLARE_VK_FUNC_PTR(vkCreateGraphicsPipelines)
  KRYS_DECLARE_VK_FUNC_PTR(vkCreateComputePipelines)
  KRYS_DECLARE_VK_FUNC_PTR(vkDestroyPipeline)

  KRYS_DECLARE_VK_FUNC_PTR(vkCreateRenderPass)
  KRYS_DECLARE_VK_FUNC_PTR(vkDestroyRenderPass)
  KRYS_DECLARE_VK_FUNC_PTR(vkCmdBeginRenderPass)
  KRYS_DECLARE_VK_FUNC_PTR(vkCmdEndRenderPass)

  KRYS_DECLARE_VK_FUNC_PTR(vkCmdNextSubpass)
  KRYS_DECLARE_VK_FUNC_PTR(vkCmdPipelineBarrier)
  KRYS_DECLARE_VK_FUNC_PTR(vkCmdSetViewport)
  KRYS_DECLARE_VK_FUNC_PTR(vkCmdSetScissor)
  KRYS_DECLARE_VK_FUNC_PTR(vkCmdBindPipeline)
  KRYS_DECLARE_VK_FUNC_PTR(vkCmdCopyBuffer)
  KRYS_DECLARE_VK_FUNC_PTR(vkCmdDraw)
  KRYS_DECLARE_VK_FUNC_PTR(vkCmdDrawIndexed)

  KRYS_DECLARE_VK_FUNC_PTR(vkCreateFramebuffer)
  KRYS_DECLARE_VK_FUNC_PTR(vkDestroyFramebuffer)

  KRYS_DECLARE_VK_FUNC_PTR(vkCreateCommandPool)
  KRYS_DECLARE_VK_FUNC_PTR(vkDestroyCommandPool)
  KRYS_DECLARE_VK_FUNC_PTR(vkResetCommandPool)
  KRYS_DECLARE_VK_FUNC_PTR(vkAllocateCommandBuffers)
  KRYS_DECLARE_VK_FUNC_PTR(vkFreeCommandBuffers)
  KRYS_DECLARE_VK_FUNC_PTR(vkBeginCommandBuffer)
  KRYS_DECLARE_VK_FUNC_PTR(vkResetCommandBuffer)
  KRYS_DECLARE_VK_FUNC_PTR(vkEndCommandBuffer)

  KRYS_DECLARE_VK_FUNC_PTR(vkCreateSemaphore)
  KRYS_DECLARE_VK_FUNC_PTR(vkDestroySemaphore)

  KRYS_DECLARE_VK_FUNC_PTR(vkCreateFence)
  KRYS_DECLARE_VK_FUNC_PTR(vkDestroyFence)
  KRYS_DECLARE_VK_FUNC_PTR(vkResetFences)
  KRYS_DECLARE_VK_FUNC_PTR(vkGetFenceStatus)
  KRYS_DECLARE_VK_FUNC_PTR(vkWaitForFences)

  KRYS_DECLARE_VK_FUNC_PTR(vkQueueSubmit)
  KRYS_DECLARE_VK_FUNC_PTR(vkQueueWaitIdle)
  KRYS_DECLARE_VK_FUNC_PTR(vkQueuePresentKHR)

  KRYS_DECLARE_VK_FUNC_PTR(vkDeviceWaitIdle)

  KRYS_DECLARE_VK_FUNC_PTR(vkCreateSwapchainKHR)
  KRYS_DECLARE_VK_FUNC_PTR(vkGetSwapchainImagesKHR)
  KRYS_DECLARE_VK_FUNC_PTR(vkDestroySwapchainKHR)
  KRYS_DECLARE_VK_FUNC_PTR(vkAcquireNextImageKHR)

  KRYS_DECLARE_VK_FUNC_PTR(vkCreateImageView)
  KRYS_DECLARE_VK_FUNC_PTR(vkDestroyImageView)

  KRYS_DECLARE_VK_FUNC_PTR(vkCreateBuffer)
  KRYS_DECLARE_VK_FUNC_PTR(vkDestroyBuffer)
  KRYS_DECLARE_VK_FUNC_PTR(vkCmdBindVertexBuffers)
  KRYS_DECLARE_VK_FUNC_PTR(vkCmdBindIndexBuffer)
  KRYS_DECLARE_VK_FUNC_PTR(vkGetBufferMemoryRequirements)
  KRYS_DECLARE_VK_FUNC_PTR(vkBindBufferMemory)

  KRYS_DECLARE_VK_FUNC_PTR(vkAllocateMemory)
  KRYS_DECLARE_VK_FUNC_PTR(vkFreeMemory)
  KRYS_DECLARE_VK_FUNC_PTR(vkMapMemory)
  KRYS_DECLARE_VK_FUNC_PTR(vkUnmapMemory)

  KRYS_DECLARE_VK_FUNC_PTR(vkCreateDescriptorSetLayout)
  KRYS_DECLARE_VK_FUNC_PTR(vkDestroyDescriptorSetLayout)
  KRYS_DECLARE_VK_FUNC_PTR(vkCreateDescriptorPool)
  KRYS_DECLARE_VK_FUNC_PTR(vkDestroyDescriptorPool)
  KRYS_DECLARE_VK_FUNC_PTR(vkResetDescriptorPool)
  KRYS_DECLARE_VK_FUNC_PTR(vkAllocateDescriptorSets)
  KRYS_DECLARE_VK_FUNC_PTR(vkFreeDescriptorSets)
  KRYS_DECLARE_VK_FUNC_PTR(vkUpdateDescriptorSets)
  KRYS_DECLARE_VK_FUNC_PTR(vkCmdBindDescriptorSets)

#undef KRYS_DECLARE_VK_FUNC_PTR
}