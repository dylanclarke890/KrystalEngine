#include "Krystal.Gfx.Vulkan/Hooks/vulkan_hooks.hpp"

namespace Krys::Gfx::Vulkan::Hooks
{
#define LOAD_VK_FUNC_PTR(name)                                                                               \
  name = reinterpret_cast<PFN_##name>(::Krys::Gfx::Vulkan::vkGetInstanceProcAddr(nullptr, #name));           \
  if (!name)                                                                                                 \
    return false;

  bool HookEntryFunctions() noexcept
  {
    LOAD_VK_FUNC_PTR(vkEnumerateInstanceExtensionProperties)
    LOAD_VK_FUNC_PTR(vkEnumerateInstanceLayerProperties)
    // LOAD_VK_FUNC_PTR(vkCreateDebugUtilsMessengerEXT)
    // LOAD_VK_FUNC_PTR(vkDestroyDebugUtilsMessengerEXT)
    // LOAD_VK_FUNC_PTR(vkSubmitDebugUtilsMessageEXT)
    return true;
  }

#undef LOAD_VK_FUNC_PTR

#define LOAD_VK_FUNC_PTR(name)                                                                               \
  name = reinterpret_cast<PFN_##name>(::Krys::Gfx::Vulkan::vkGetInstanceProcAddr(instance, #name));          \
  if (!name)                                                                                                 \
    return false;

  bool HookInstanceFunctions(VkInstance instance) noexcept
  {
    if (!instance)
      return false;

    LOAD_VK_FUNC_PTR(vkCreateInstance)
    LOAD_VK_FUNC_PTR(vkDestroyInstance)
    LOAD_VK_FUNC_PTR(vkCreateDevice)
    LOAD_VK_FUNC_PTR(vkGetDeviceProcAddr)
    LOAD_VK_FUNC_PTR(vkEnumeratePhysicalDevices)
    LOAD_VK_FUNC_PTR(vkGetPhysicalDeviceProperties)
    LOAD_VK_FUNC_PTR(vkGetPhysicalDeviceFeatures)
    LOAD_VK_FUNC_PTR(vkGetPhysicalDeviceFormatProperties)
    LOAD_VK_FUNC_PTR(vkGetPhysicalDeviceImageFormatProperties)
    LOAD_VK_FUNC_PTR(vkGetPhysicalDeviceQueueFamilyProperties)
    LOAD_VK_FUNC_PTR(vkGetPhysicalDeviceMemoryProperties)
    LOAD_VK_FUNC_PTR(vkEnumeratePhysicalDeviceGroups)
    LOAD_VK_FUNC_PTR(vkEnumerateDeviceExtensionProperties)
    LOAD_VK_FUNC_PTR(vkGetPhysicalDeviceExternalBufferProperties)
    LOAD_VK_FUNC_PTR(vkGetPhysicalDeviceExternalFenceProperties)
    LOAD_VK_FUNC_PTR(vkGetPhysicalDeviceExternalSemaphoreProperties)

    LOAD_VK_FUNC_PTR(vkDestroySurfaceKHR)
    LOAD_VK_FUNC_PTR(vkGetPhysicalDeviceSurfaceSupportKHR)
    LOAD_VK_FUNC_PTR(vkGetPhysicalDeviceSurfaceCapabilitiesKHR)
    LOAD_VK_FUNC_PTR(vkGetPhysicalDeviceSurfaceFormatsKHR)
    LOAD_VK_FUNC_PTR(vkGetPhysicalDeviceSurfacePresentModesKHR)

    return true;
  }

  bool HookDebugMessengerFunctions(VkInstance instance) noexcept
  {
    if (!instance)
      return false;

    LOAD_VK_FUNC_PTR(vkCreateDebugUtilsMessengerEXT)
    LOAD_VK_FUNC_PTR(vkDestroyDebugUtilsMessengerEXT)
    LOAD_VK_FUNC_PTR(vkSubmitDebugUtilsMessageEXT)

    return true;
  }

#undef LOAD_VK_FUNC_PTR

  bool HookDeviceFunctions(VkDevice device) noexcept
  {
#define LOAD_VK_FUNC_PTR(name)                                                                               \
  name = reinterpret_cast<PFN_##name>(vkGetDeviceProcAddr(device, #name));                                   \
  if (!name)                                                                                                 \
    return false;

    if (!device)
      return false;

    LOAD_VK_FUNC_PTR(vkDestroyDevice)
    LOAD_VK_FUNC_PTR(vkGetDeviceQueue)

    LOAD_VK_FUNC_PTR(vkCreateShaderModule)
    LOAD_VK_FUNC_PTR(vkDestroyShaderModule)

    LOAD_VK_FUNC_PTR(vkCreatePipelineLayout)
    LOAD_VK_FUNC_PTR(vkDestroyPipelineLayout)

    LOAD_VK_FUNC_PTR(vkCreateGraphicsPipelines)
    LOAD_VK_FUNC_PTR(vkCreateComputePipelines)
    LOAD_VK_FUNC_PTR(vkDestroyPipeline)

    LOAD_VK_FUNC_PTR(vkCreateRenderPass)
    LOAD_VK_FUNC_PTR(vkDestroyRenderPass)
    LOAD_VK_FUNC_PTR(vkCmdBeginRenderPass)
    LOAD_VK_FUNC_PTR(vkCmdEndRenderPass)

    LOAD_VK_FUNC_PTR(vkCmdNextSubpass)
    LOAD_VK_FUNC_PTR(vkCmdPipelineBarrier)
    LOAD_VK_FUNC_PTR(vkCmdSetViewport)
    LOAD_VK_FUNC_PTR(vkCmdSetScissor)
    LOAD_VK_FUNC_PTR(vkCmdBindPipeline)
    LOAD_VK_FUNC_PTR(vkCmdCopyBuffer)
    LOAD_VK_FUNC_PTR(vkCmdDraw)
    LOAD_VK_FUNC_PTR(vkCmdDrawIndexed)

    LOAD_VK_FUNC_PTR(vkCreateFramebuffer)
    LOAD_VK_FUNC_PTR(vkDestroyFramebuffer)

    LOAD_VK_FUNC_PTR(vkCreateCommandPool)
    LOAD_VK_FUNC_PTR(vkDestroyCommandPool)
    LOAD_VK_FUNC_PTR(vkResetCommandPool)
    LOAD_VK_FUNC_PTR(vkAllocateCommandBuffers)
    LOAD_VK_FUNC_PTR(vkFreeCommandBuffers)
    LOAD_VK_FUNC_PTR(vkResetCommandBuffer)
    LOAD_VK_FUNC_PTR(vkBeginCommandBuffer)
    LOAD_VK_FUNC_PTR(vkEndCommandBuffer)

    LOAD_VK_FUNC_PTR(vkCreateSemaphore)
    LOAD_VK_FUNC_PTR(vkDestroySemaphore)

    LOAD_VK_FUNC_PTR(vkCreateFence)
    LOAD_VK_FUNC_PTR(vkDestroyFence)
    LOAD_VK_FUNC_PTR(vkResetFences)
    LOAD_VK_FUNC_PTR(vkGetFenceStatus)
    LOAD_VK_FUNC_PTR(vkWaitForFences)

    LOAD_VK_FUNC_PTR(vkQueueSubmit)
    LOAD_VK_FUNC_PTR(vkQueueWaitIdle)
    LOAD_VK_FUNC_PTR(vkQueuePresentKHR)

    LOAD_VK_FUNC_PTR(vkDeviceWaitIdle)

    LOAD_VK_FUNC_PTR(vkCreateSwapchainKHR)
    LOAD_VK_FUNC_PTR(vkGetSwapchainImagesKHR)
    LOAD_VK_FUNC_PTR(vkDestroySwapchainKHR)
    LOAD_VK_FUNC_PTR(vkAcquireNextImageKHR)

    LOAD_VK_FUNC_PTR(vkCreateImageView)
    LOAD_VK_FUNC_PTR(vkDestroyImageView)

    LOAD_VK_FUNC_PTR(vkCreateBuffer)
    LOAD_VK_FUNC_PTR(vkDestroyBuffer)
    LOAD_VK_FUNC_PTR(vkGetBufferMemoryRequirements)
    LOAD_VK_FUNC_PTR(vkBindBufferMemory)
    LOAD_VK_FUNC_PTR(vkCmdBindVertexBuffers)
    LOAD_VK_FUNC_PTR(vkCmdBindIndexBuffer)

    LOAD_VK_FUNC_PTR(vkAllocateMemory)
    LOAD_VK_FUNC_PTR(vkFreeMemory)
    LOAD_VK_FUNC_PTR(vkMapMemory)
    LOAD_VK_FUNC_PTR(vkUnmapMemory)

    LOAD_VK_FUNC_PTR(vkCreateDescriptorSetLayout)
    LOAD_VK_FUNC_PTR(vkDestroyDescriptorSetLayout)
    LOAD_VK_FUNC_PTR(vkCreateDescriptorPool)
    LOAD_VK_FUNC_PTR(vkDestroyDescriptorPool)
    LOAD_VK_FUNC_PTR(vkResetDescriptorPool)
    LOAD_VK_FUNC_PTR(vkAllocateDescriptorSets)
    LOAD_VK_FUNC_PTR(vkFreeDescriptorSets)
    LOAD_VK_FUNC_PTR(vkUpdateDescriptorSets)
    LOAD_VK_FUNC_PTR(vkCmdBindDescriptorSets)

#undef LOAD_VK_FUNC_PTR

    return true;
  }
}