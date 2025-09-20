#include "Krystal.Gfx.Vulkan/Hooks/vulkan_win32_hooks.hpp"
#include <stdexcept>

namespace Krys::Gfx::Vulkan
{
  static HMODULE VulkanLibrary {};

  namespace Hooks
  {
    bool HookCreateInstanceFunction() noexcept
    {
      HMODULE VulkanLibrary = LoadLibraryA("vulkan-1.dll");
      if (!VulkanLibrary)
        return false;

      vkGetInstanceProcAddr =
        reinterpret_cast<PFN_vkGetInstanceProcAddr>(GetProcAddress(VulkanLibrary, "vkGetInstanceProcAddr"));
      if (vkGetInstanceProcAddr == nullptr)
        return false;

      vkCreateInstance =
        reinterpret_cast<PFN_vkCreateInstance>(vkGetInstanceProcAddr(VK_NULL_HANDLE, "vkCreateInstance"));
      return vkCreateInstance != nullptr;
    }

#define LOAD_VK_FUNC_PTR(name)                                                                               \
  name = reinterpret_cast<PFN_##name>(vkGetInstanceProcAddr(instance, #name));                               \
  if (!name)                                                                                                 \
    return false;

    bool HookPlatformInstanceFunctions(VkInstance instance) noexcept
    {
      if (!instance)
        return false;

      LOAD_VK_FUNC_PTR(vkCreateWin32SurfaceKHR)
      // LOAD_VK_FUNC_PTR(GetPhysicalDeviceWin32PresentationSupportKHR)
      // LOAD_VK_FUNC_PTR(GetMemoryWin32HandleKHR)
      // LOAD_VK_FUNC_PTR(GetMemoryWin32HandlePropertiesKHR)
      // LOAD_VK_FUNC_PTR(ImportSemaphoreWin32HandleKHR)
      // LOAD_VK_FUNC_PTR(GetSemaphoreWin32HandleKHR)
      // LOAD_VK_FUNC_PTR(ImportFenceWin32HandleKHR)
      // LOAD_VK_FUNC_PTR(GetFenceWin32HandleKHR)
      // LOAD_VK_FUNC_PTR(GetMemoryWin32HandleNV)
      // LOAD_VK_FUNC_PTR(GetPhysicalDeviceSurfacePresentModes2EXT)
      // LOAD_VK_FUNC_PTR(AcquireFullScreenExclusiveModeEXT)
      // LOAD_VK_FUNC_PTR(ReleaseFullScreenExclusiveModeEXT)
      // LOAD_VK_FUNC_PTR(GetDeviceGroupSurfacePresentModes2EXT)
      // LOAD_VK_FUNC_PTR(AcquireWinrtDisplayNV)
      // LOAD_VK_FUNC_PTR(GetWinrtDisplayNV)
      return true;
    }

#undef LOAD_VK_FUNC_PTR

  }

  VkSurfaceKHR CreateSurface(VkInstance instance, NativeHandle windowHandle)
  {
    VkSurfaceKHR surface {};

    VkWin32SurfaceCreateInfoKHR createInfo {
      .sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
      .pNext = nullptr,
      .flags = 0,
      .hinstance = GetModuleHandle(NULL),
      .hwnd = windowHandle.As<HWND>(),
    };

    if (vkCreateWin32SurfaceKHR(instance, &createInfo, nullptr, &surface) != VK_SUCCESS)
    {
      throw std::runtime_error("Failed to create Vulkan surface");
    }

    return surface;
  }
}
