#pragma once

#include "Krystal.Core/Core.hpp"
#include "Krystal.Core/Detection.hpp"

#include "Krystal.Gfx.Vulkan/Hooks/vulkan_hooks.hpp"

#ifndef WIN32_LEAN_AND_MEAN
  #define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#undef MIN
#undef min
#undef MAX
#undef max

#include <vulkan/vulkan_win32.h>

namespace Krys::Gfx::Vulkan
{
#define DECLARE_VK_FUNC_PTR(name) inline PFN_##name name = nullptr;

  DECLARE_VK_FUNC_PTR(vkCreateWin32SurfaceKHR)
  //DECLARE_VK_FUNC_PTR(vkGetPhysicalDeviceWin32PresentationSupportKHR)
  //DECLARE_VK_FUNC_PTR(vkGetMemoryWin32HandleKHR)
  //DECLARE_VK_FUNC_PTR(vkGetMemoryWin32HandlePropertiesKHR)
  //DECLARE_VK_FUNC_PTR(vkImportSemaphoreWin32HandleKHR)
  //DECLARE_VK_FUNC_PTR(vkGetSemaphoreWin32HandleKHR)
  //DECLARE_VK_FUNC_PTR(vkImportFenceWin32HandleKHR)
  //DECLARE_VK_FUNC_PTR(vkGetFenceWin32HandleKHR)
  //DECLARE_VK_FUNC_PTR(vkGetMemoryWin32HandleNV)
  //DECLARE_VK_FUNC_PTR(vkGetPhysicalDeviceSurfacePresentModes2EXT)
  //DECLARE_VK_FUNC_PTR(vkAcquireFullScreenExclusiveModeEXT)
  //DECLARE_VK_FUNC_PTR(vkReleaseFullScreenExclusiveModeEXT)
  //DECLARE_VK_FUNC_PTR(vkGetDeviceGroupSurfacePresentModes2EXT)
  //DECLARE_VK_FUNC_PTR(vkAcquireWinrtDisplayNV)
  //DECLARE_VK_FUNC_PTR(vkGetWinrtDisplayNV)

#undef DECLARE_VK_FUNC_PTR
}