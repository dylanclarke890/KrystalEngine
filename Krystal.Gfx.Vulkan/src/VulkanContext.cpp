#include "Krystal.Gfx.Vulkan/VulkanContext.hpp"

#include "Krystal.Lib/Detection.hpp"

#ifdef KRYS_PLATFORM_WINDOWS
  #include "Krystal.Gfx.Vulkan/Hooks/vulkan_win32_hooks.hpp"
  #define KRYS_SURFACE_EXTENSION_NAME VK_KHR_WIN32_SURFACE_EXTENSION_NAME
#endif

#include "Krystal.IO/Streams/NativeFileStream.hpp"
#include "Krystal.IO/Streams/StreamUtils.hpp"
#include "Krystal.Lib/Array.hpp"
#include "Krystal.Maths/Clipspace.hpp"
#include "Krystal.Maths/Convert.hpp"
#include "Krystal.Maths/Matrix.hpp"
#include "Krystal.Maths/Transform.hpp"
#include "Krystal.Maths/Vector.hpp"
#include <chrono>
#include <iostream>
#include <ranges>

namespace Krys::Gfx
{
  // Expected<Unique<IContext>> CreateContext(NativeHandle windowHandle, uint32 width, uint32 height) noexcept
  //{
  //   try
  //   {
  //     return Expected<Unique<IContext>>(CreateUnique<Vulkan::VulkanContext>(windowHandle, width, height));
  //   }
  //   catch (const std::exception &e)
  //   {
  //     return Unexpected(e.what());
  //   }
  // }
}

namespace
{
  Krys::List<const char *> InstanceExtensions {VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
                                               VK_KHR_SURFACE_EXTENSION_NAME, KRYS_SURFACE_EXTENSION_NAME};

  Krys::List<const char *> DeviceExtensions {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

  Krys::List<const char *> ValidationLayers {"VK_LAYER_KHRONOS_validation"};

  struct Vertex
  {
    Krys::Maths::Vec2 Position;
    Krys::Maths::Vec3 Color;

    static VkVertexInputBindingDescription GetBindingDescription()
    {
      return VkVertexInputBindingDescription {
        .binding = 0, .stride = sizeof(Vertex), .inputRate = VK_VERTEX_INPUT_RATE_VERTEX};
    }

    static Krys::Array<VkVertexInputAttributeDescription, 2> GetAttributeDescriptions()
    {
      return Krys::Array<VkVertexInputAttributeDescription, 2> {VkVertexInputAttributeDescription {
                                                                  .location = 0,
                                                                  .binding = 0,
                                                                  .format = VK_FORMAT_R32G32_SFLOAT,
                                                                  .offset = offsetof(Vertex, Position),
                                                                },
                                                                VkVertexInputAttributeDescription {
                                                                  .location = 1,
                                                                  .binding = 0,
                                                                  .format = VK_FORMAT_R32G32B32_SFLOAT,
                                                                  .offset = offsetof(Vertex, Color),
                                                                }};
    }
  };

  struct UniformBufferObject
  {
    alignas(16) Krys::Maths::Mat4 model;
    alignas(16) Krys::Maths::Mat4 view;
    alignas(16) Krys::Maths::Mat4 proj;
  };

  const Krys::List<Vertex> vertices = {{{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
                                       {{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
                                       {{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
                                       {{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}}};

  const Krys::List<Krys::uint16> indices = {0, 1, 2, 2, 3, 0};

#ifdef KRYS_BUILD_DEBUG
  bool EnableValidationLayers = false;
#else
  bool EnableValidationLayers = true;
#endif

  const int MAX_FRAMES_IN_FLIGHT = 3;

  static VKAPI_ATTR VkBool32 VKAPI_CALL DebugMessageCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData, void *pUserData)
  {
    // TODO: use the engine logger
    std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

    return VK_FALSE;
  }

  Krys::Expected<Krys::List<Krys::uint32>> LoadShaderCode(const Krys::string &filePath) noexcept
  {
    using Reader = Krys::IO::NativeFileReader;
    Reader fileStream {Krys::IO::Path(filePath)};

    return Krys::IO::StreamUtils::ReadAllAs<Krys::uint32>(fileStream);
  }

  VkShaderModule CreateShaderModule(VkDevice device, const Krys::List<Krys::uint32> &code) noexcept
  {
    using namespace Krys::Gfx::Vulkan;

    VkShaderModuleCreateInfo createInfo {
      .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0,
      .codeSize = code.size() * sizeof(Krys::uint32),
      .pCode = code.data(),
    };

    VkShaderModule shaderModule;
    vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule);

    return shaderModule;
  }
}

namespace Krys::Gfx::Vulkan
{
  VulkanContext::VulkanContext(const ContextSettings &settings)
      : _windowHandle(settings.WindowHandle), _width(settings.Width), _height(settings.Height),
        _vfs(*settings.VFS)
  {
    CreateInstance();
    SetupDebugMessenger();
    _surface = CreateSurface(_instance, _windowHandle);
    SelectPhysicalDevice();
    CreateDevice();
    CreateSwapchain();
    CreateSwapchainImageViews();
    CreateRenderPass();
    CreateDescriptorSetLayout();
    CreateGraphicsPipeline();
    CreateFramebuffers();
    CreateCommandPool();
    CreateVertexBuffer();
    CreateIndexBuffer();
    CreateUniformBuffers();
    CreateDescriptorPool();
    CreateDescriptorSets();
    CreateCommandBuffers();
    CreateSyncObjects();
  }

  VulkanContext::~VulkanContext() noexcept
  {
    vkDeviceWaitIdle(_device);
    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
    {
      vkDestroySemaphore(_device, _imageAvailableSemaphores[i], nullptr);
      vkDestroySemaphore(_device, _renderFinishedSemaphores[i], nullptr);
      vkDestroyFence(_device, _inFlightFences[i], nullptr);
    }
    vkDestroyCommandPool(_device, _commandPool, nullptr);

    CleanupSwapchain();

    vkDestroyDescriptorPool(_device, _descriptorPool, nullptr);
    vkDestroyDescriptorSetLayout(_device, _descriptorSetLayout, nullptr);

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
      vkDestroyBuffer(_device, _uniformBuffers[i], nullptr);
      vkFreeMemory(_device, _uniformBuffersMemory[i], nullptr);
    }

    vkDestroyBuffer(_device, _vertexBuffer, nullptr);
    vkFreeMemory(_device, _vertexBufferMemory, nullptr);
    vkDestroyBuffer(_device, _indexBuffer, nullptr);
    vkFreeMemory(_device, _indexBufferMemory, nullptr);

    vkDestroyPipeline(_device, _pipeline, nullptr);
    vkDestroyPipelineLayout(_device, _pipelineLayout, nullptr);
    vkDestroyRenderPass(_device, _renderPass, nullptr);

    vkDestroyDevice(_device, nullptr);
    vkDestroySurfaceKHR(_instance, _surface, nullptr);
    if (_debugMessenger != VK_NULL_HANDLE)
      vkDestroyDebugUtilsMessengerEXT(_instance, _debugMessenger, nullptr);
    vkDestroyInstance(_instance, nullptr);
  }

#pragma region Initialisation

  void VulkanContext::CreateInstance()
  {
    if (!Hooks::HookCreateInstanceFunction())
    {
      throw std::runtime_error("Failed to load vkCreateInstance function.");
    }

    if (!Hooks::HookEntryFunctions())
    {
      throw std::runtime_error("Failed to hook Vulkan entry functions.");
    }

    VkApplicationInfo application {.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
                                   .pNext = nullptr,
                                   // TODO: get application name
                                   .pApplicationName = "Krystal Gfx Vulkan",
                                   // TODO: get application version
                                   .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
                                   // TODO: get engine name
                                   .pEngineName = "Krystal Engine",
                                   // TODO: get engine version
                                   .engineVersion = VK_MAKE_VERSION(1, 0, 0),
                                   // TODO: set api version
                                   .apiVersion = VK_API_VERSION_1_0};

    if (EnableValidationLayers && !AreValidationLayersAvailable())
    {
      throw std::runtime_error("validation layers requested, but not available!");
    }

    VkInstanceCreateInfo instanceCreateInfo {
      .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0,
      .pApplicationInfo = &application,
      .enabledLayerCount = 0,
      .ppEnabledLayerNames = nullptr,
      .enabledExtensionCount = static_cast<uint32>(InstanceExtensions.size()),
      .ppEnabledExtensionNames = InstanceExtensions.data(),
    };

    if (EnableValidationLayers)
    {
      instanceCreateInfo.enabledLayerCount = static_cast<uint32_t>(ValidationLayers.size());
      instanceCreateInfo.ppEnabledLayerNames = ValidationLayers.data();
    }

    if (vkCreateInstance(&instanceCreateInfo, nullptr, &_instance) != VK_SUCCESS)
    {
      throw std::runtime_error("Failed to create Vulkan instance");
    }

    if (!Hooks::HookInstanceFunctions(_instance) || !Hooks::HookPlatformInstanceFunctions(_instance))
    {
      if (_instance != VK_NULL_HANDLE)
      {
        vkDestroyInstance(_instance, nullptr);
      }
      throw std::runtime_error("Failed to initialise Vulkan hooks");
    }
  }

  void VulkanContext::SetupDebugMessenger()
  {
    if (!EnableValidationLayers)
    {
      return;
    }

    if (!Hooks::HookDebugMessengerFunctions(_instance))
    {
      throw std::runtime_error("Failed to hook debug messenger functions");
    }

    VkDebugUtilsMessengerCreateInfoEXT createInfo {
      .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
      .pNext = nullptr,
      .flags = 0,
      .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT
                         | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
                         | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
      .messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
                     | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
                     | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
      .pfnUserCallback = DebugMessageCallback,
      .pUserData = nullptr};

    if (vkCreateDebugUtilsMessengerEXT(_instance, &createInfo, nullptr, &_debugMessenger) != VK_SUCCESS)
    {
      throw std::runtime_error("Failed to set up debug messenger");
    }
  }

  bool VulkanContext::AreValidationLayersAvailable()
  {
    uint32 layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    List<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    for (const char *layerName : ValidationLayers)
    {
      bool layerFound = false;

      for (const auto &layerProperties : availableLayers)
      {
        if (strcmp(layerName, layerProperties.layerName) == 0)
        {
          layerFound = true;
          break;
        }
      }

      if (!layerFound)
      {
        return false;
      }
    }

    return true;
  }

  void VulkanContext::SelectPhysicalDevice()
  {
    uint32 deviceCount = 0;
    if (vkEnumeratePhysicalDevices(_instance, &deviceCount, nullptr) != VK_SUCCESS)
    {
      throw std::runtime_error("Failed to enumerate physical devices");
    }

    if (deviceCount == 0)
    {
      throw std::runtime_error("No Vulkan-compatible devices found");
    }

    List<VkPhysicalDevice> devices(deviceCount);
    if (vkEnumeratePhysicalDevices(_instance, &deviceCount, devices.data()) != VK_SUCCESS)
    {
      throw std::runtime_error("Failed to enumerate physical devices");
    }

    for (const auto &device : devices)
    {
      if (IsDeviceSuitable(device))
      {
        _physicalDevice = device;
        break;
      }
    }
  }

  bool VulkanContext::IsDeviceSuitable(VkPhysicalDevice device)
  {
    auto indices = FindQueueFamilyIndices(device);
    if (!indices.has_value() || !AreDeviceExtensionsSupported(device))
    {
      return false;
    }

    auto swapchainSupport = QuerySwapchainSupport(device);
    if (swapchainSupport.Formats.empty() || swapchainSupport.PresentModes.empty())
    {
      return false;
    }

    _queueFamilyIndices = indices.value();

    return true;
  }

  Nullable<QueueFamilyIndices> VulkanContext::FindQueueFamilyIndices(VkPhysicalDevice device) const
  {
    QueueFamilyIndices indices;

    uint32 queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
    if (queueFamilyCount == 0)
    {
      throw std::runtime_error("No queue families found for the physical device");
    }

    List<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

    VkBool32 isGraphicsFamilyFound = VK_FALSE;
    VkBool32 isPresentFamilyFound = VK_FALSE;
    auto IsComplete = [&]()
    {
      return isGraphicsFamilyFound && isPresentFamilyFound;
    };

    for (uint32 i = 0; i < queueFamilyCount; ++i)
    {
      const auto &queueFamily = queueFamilies[i];
      if (!isGraphicsFamilyFound && queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
      {
        indices.GraphicsFamily = i;
        isGraphicsFamilyFound = VK_TRUE;
      }

      if (!isPresentFamilyFound)
      {
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, _surface, &isPresentFamilyFound);
        if (isPresentFamilyFound)
          indices.PresentFamily = i;
      }

      if (IsComplete())
        break;
    }

    if (IsComplete())
      return indices;
    return std::nullopt;
  }

  bool VulkanContext::AreDeviceExtensionsSupported(VkPhysicalDevice device) const noexcept
  {
    uint32 extensionCount;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

    List<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

    Set<string> requiredExtensions(DeviceExtensions.begin(), DeviceExtensions.end());
    for (const auto &extension : availableExtensions)
    {
      requiredExtensions.erase(extension.extensionName);
    }

    return requiredExtensions.empty();
  }

  SwapchainSupportDetails VulkanContext::QuerySwapchainSupport(VkPhysicalDevice device) const noexcept
  {
    SwapchainSupportDetails details;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, _surface, &details.SurfaceCapabilities);

    uint32 formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, _surface, &formatCount, nullptr);
    if (formatCount != 0)
    {
      details.Formats.resize(formatCount);
      vkGetPhysicalDeviceSurfaceFormatsKHR(device, _surface, &formatCount, details.Formats.data());
    }

    uint32 presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, _surface, &presentModeCount, nullptr);
    if (presentModeCount != 0)
    {
      details.PresentModes.resize(presentModeCount);
      vkGetPhysicalDeviceSurfacePresentModesKHR(device, _surface, &presentModeCount,
                                                details.PresentModes.data());
    }

    return details;
  }

  void VulkanContext::CreateDevice()
  {
    List<VkDeviceQueueCreateInfo> queueCreateInfos;
    Set<uint32> uniqueQueueFamilies = {_queueFamilyIndices.GraphicsFamily, _queueFamilyIndices.PresentFamily};

    float queuePriority = 1.0f;
    for (uint32_t queueFamily : uniqueQueueFamilies)
    {
      VkDeviceQueueCreateInfo queueCreateInfo {};
      queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
      queueCreateInfo.queueFamilyIndex = queueFamily;
      queueCreateInfo.queueCount = 1;
      queueCreateInfo.pQueuePriorities = &queuePriority;
      queueCreateInfos.push_back(queueCreateInfo);
    }

    VkPhysicalDeviceFeatures deviceFeatures {};
    VkDeviceCreateInfo createInfo {
      .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0,
      .queueCreateInfoCount = static_cast<uint32>(queueCreateInfos.size()),
      .pQueueCreateInfos = queueCreateInfos.data(),
      .enabledLayerCount = 0,
      .ppEnabledLayerNames = nullptr,
      .enabledExtensionCount = static_cast<uint32>(DeviceExtensions.size()),
      .ppEnabledExtensionNames = DeviceExtensions.data(),
      .pEnabledFeatures = &deviceFeatures,
    };

    if (vkCreateDevice(_physicalDevice, &createInfo, nullptr, &_device) != VK_SUCCESS)
    {
      throw std::runtime_error("Failed to create Vulkan logical device");
    }

    if (!Hooks::HookDeviceFunctions(_device))
    {
      vkDestroyDevice(_device, nullptr);
      throw std::runtime_error("Failed to initialize Vulkan hooks for device");
    }

    vkGetDeviceQueue(_device, _queueFamilyIndices.GraphicsFamily, 0, &_graphicsQueue);
    vkGetDeviceQueue(_device, _queueFamilyIndices.PresentFamily, 0, &_presentQueue);
  }

  void VulkanContext::CreateSwapchain()
  {
    SwapchainSupportDetails swapchainSupport = QuerySwapchainSupport(_physicalDevice);

    VkSurfaceFormatKHR surfaceFormat = ChooseSwapSurfaceFormat(swapchainSupport.Formats);
    VkPresentModeKHR presentMode = ChooseSwapPresentMode(swapchainSupport.PresentModes);
    VkExtent2D extent = ChooseSwapExtent(swapchainSupport.SurfaceCapabilities);

    uint32 imageCount = swapchainSupport.SurfaceCapabilities.minImageCount + 1;
    if (swapchainSupport.SurfaceCapabilities.maxImageCount > 0
        && imageCount > swapchainSupport.SurfaceCapabilities.maxImageCount)
    {
      imageCount = swapchainSupport.SurfaceCapabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR createInfo {
      .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
      .pNext = nullptr,
      .flags = 0,
      .surface = _surface,
      .minImageCount = imageCount,
      .imageFormat = surfaceFormat.format,
      .imageColorSpace = surfaceFormat.colorSpace,
      .imageExtent = extent,
      .imageArrayLayers = 1,
      .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
      .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
      .queueFamilyIndexCount = 0,
      .pQueueFamilyIndices = nullptr,
      .preTransform = swapchainSupport.SurfaceCapabilities.currentTransform,
      .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
      .presentMode = presentMode,
      .clipped = VK_TRUE,
      .oldSwapchain = VK_NULL_HANDLE,
    };

    Set<uint32> queueFamilyIndices = {_queueFamilyIndices.GraphicsFamily, _queueFamilyIndices.PresentFamily};
    List<uint32> queueFamilyIndicesList(queueFamilyIndices.begin(), queueFamilyIndices.end());

    if (queueFamilyIndices.size() > 1)
    {
      createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
      createInfo.queueFamilyIndexCount = static_cast<uint32>(queueFamilyIndices.size());
      createInfo.pQueueFamilyIndices = queueFamilyIndicesList.data();
    }

    if (vkCreateSwapchainKHR(_device, &createInfo, nullptr, &_swapchain) != VK_SUCCESS)
    {
      throw std::runtime_error("Failed to create swapchain");
    }

    vkGetSwapchainImagesKHR(_device, _swapchain, &imageCount, nullptr);
    _swapchainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(_device, _swapchain, &imageCount, _swapchainImages.data());

    _swapchainImageFormat = surfaceFormat.format;
    _swapchainExtent = extent;
  }

  VkSurfaceFormatKHR
    VulkanContext::ChooseSwapSurfaceFormat(const List<VkSurfaceFormatKHR> &availableFormats) const noexcept
  {
    const auto &bestMatch =
      std::ranges::find_if(availableFormats,
                           [](const VkSurfaceFormatKHR &format)
                           {
                             return format.format == VK_FORMAT_B8G8R8A8_SRGB
                                    && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
                           });
    if (bestMatch != availableFormats.end())
    {
      return *bestMatch;
    }

    return availableFormats[0];
  }

  VkPresentModeKHR
    VulkanContext::ChooseSwapPresentMode(const List<VkPresentModeKHR> &availablePresentModes) const noexcept
  {
    const auto &bestMatch = std::ranges::find(availablePresentModes, VK_PRESENT_MODE_MAILBOX_KHR);
    if (bestMatch != availablePresentModes.end())
    {
      return *bestMatch;
    }
    return VK_PRESENT_MODE_FIFO_KHR;
  }

  VkExtent2D VulkanContext::ChooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities) const noexcept
  {
    if (capabilities.currentExtent.width != std::numeric_limits<uint32>::max())
    {
      return capabilities.currentExtent;
    }

    VkExtent2D actualExtent = {_width, _height};
    actualExtent.width =
      std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
    actualExtent.height =
      std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

    return actualExtent;
  }

  void VulkanContext::CreateSwapchainImageViews()
  {
    _swapchainImageViews.resize(_swapchainImages.size());
    for (const auto &[i, image] : _swapchainImages | std::views::enumerate)
    {
      VkImageViewCreateInfo viewInfo {.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
                                      .pNext = nullptr,
                                      .flags = 0,
                                      .image = _swapchainImages[i],
                                      .viewType = VK_IMAGE_VIEW_TYPE_2D,
                                      .format = _swapchainImageFormat,
                                      .components = {.r = VK_COMPONENT_SWIZZLE_IDENTITY,
                                                     .g = VK_COMPONENT_SWIZZLE_IDENTITY,
                                                     .b = VK_COMPONENT_SWIZZLE_IDENTITY,
                                                     .a = VK_COMPONENT_SWIZZLE_IDENTITY},
                                      .subresourceRange = {.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                                                           .baseMipLevel = 0,
                                                           .levelCount = 1,
                                                           .baseArrayLayer = 0,
                                                           .layerCount = 1}};
      if (vkCreateImageView(_device, &viewInfo, nullptr, &_swapchainImageViews[i]) != VK_SUCCESS)
      {
        throw std::runtime_error("Failed to create swap chain image views");
      }
    }
  }

#pragma endregion

  void VulkanContext::RecreateSwapchain()
  {
    vkDeviceWaitIdle(_device);

    _framebufferResized = false;
    CleanupSwapchain();

    CreateSwapchain();
    CreateSwapchainImageViews();
    CreateFramebuffers();
  }

  void VulkanContext::CleanupSwapchain()
  {
    for (const auto &framebuffer : _swapchainFramebuffers)
      vkDestroyFramebuffer(_device, framebuffer, nullptr);
    for (auto imageView : _swapchainImageViews)
      vkDestroyImageView(_device, imageView, nullptr);
    vkDestroySwapchainKHR(_device, _swapchain, nullptr);
  }

  void VulkanContext::CreateRenderPass() noexcept
  {
    VkAttachmentReference colorAttachmentRef {
      .attachment = 0,
      .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
    };

    VkSubpassDescription subpass {
      .flags = 0,
      .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
      .inputAttachmentCount = 0,
      .pInputAttachments = nullptr,
      .colorAttachmentCount = 1,
      .pColorAttachments = &colorAttachmentRef,
      .pResolveAttachments = nullptr,
      .pDepthStencilAttachment = nullptr,
      .preserveAttachmentCount = 0,
      .pPreserveAttachments = nullptr,
    };

    VkAttachmentDescription colorAttachment {
      .flags = 0,
      .format = _swapchainImageFormat,
      .samples = VK_SAMPLE_COUNT_1_BIT,
      .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
      .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
      .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
      .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
      .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
      .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
    };

    VkSubpassDependency dependency {
      .srcSubpass = VK_SUBPASS_EXTERNAL,
      .dstSubpass = 0,
      .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
      .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
      .srcAccessMask = 0,
      .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
      .dependencyFlags = 0,
    };

    VkRenderPassCreateInfo renderPassInfo {
      .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0,
      .attachmentCount = 1,
      .pAttachments = &colorAttachment,
      .subpassCount = 1,
      .pSubpasses = &subpass,
      .dependencyCount = 1,
      .pDependencies = &dependency,
    };

    vkCreateRenderPass(_device, &renderPassInfo, nullptr, &_renderPass);
  }

  void VulkanContext::CreateDescriptorSetLayout()
  {
    VkDescriptorSetLayoutBinding uboLayoutBinding {
      .binding = 0,
      .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
      .descriptorCount = 1,
      .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
      .pImmutableSamplers = nullptr,
    };

    VkDescriptorSetLayoutCreateInfo layoutInfo {
      .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0,
      .bindingCount = 1,
      .pBindings = &uboLayoutBinding,
    };

    if (vkCreateDescriptorSetLayout(_device, &layoutInfo, nullptr, &_descriptorSetLayout) != VK_SUCCESS)
    {
      throw std::runtime_error("Failed to create descriptor set layout");
    }
  }

  void VulkanContext::CreateGraphicsPipeline()
  {
    auto vertexShaderModule =
      CreateShaderModule(_device, LoadShaderCode("build/shaders/triangle.vert.spv").value());
    auto fragmentShaderModule =
      CreateShaderModule(_device, LoadShaderCode("build/shaders/triangle.frag.spv").value());

    VkPipelineShaderStageCreateInfo shaderStages[2] = {
      {.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
       .pNext = nullptr,
       .flags = 0,
       .stage = VK_SHADER_STAGE_VERTEX_BIT,
       .module = vertexShaderModule,
       .pName = "main",
       .pSpecializationInfo = nullptr},
      {.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
       .pNext = nullptr,
       .flags = 0,
       .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
       .module = fragmentShaderModule,
       .pName = "main",
       .pSpecializationInfo = nullptr}};

    List<VkDynamicState> dynamicStates = {
      VK_DYNAMIC_STATE_VIEWPORT,
      VK_DYNAMIC_STATE_SCISSOR,
    };
    VkPipelineDynamicStateCreateInfo dynamicState {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0,
      .dynamicStateCount = static_cast<uint32>(dynamicStates.size()),
      .pDynamicStates = dynamicStates.data(),
    };

    VkPipelineViewportStateCreateInfo viewportState {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0,
      .viewportCount = 1,
      .pViewports = nullptr,
      .scissorCount = 1,
      .pScissors = nullptr,
    };

    auto vertexBindingDescription = Vertex::GetBindingDescription();
    auto vertexAttributeDescriptions = Vertex::GetAttributeDescriptions();

    VkPipelineVertexInputStateCreateInfo vertexInputState {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0,
      .vertexBindingDescriptionCount = 1,
      .pVertexBindingDescriptions = &vertexBindingDescription,
      .vertexAttributeDescriptionCount = static_cast<uint32>(vertexAttributeDescriptions.size()),
      .pVertexAttributeDescriptions = vertexAttributeDescriptions.data(),
    };

    VkPipelineInputAssemblyStateCreateInfo inputAssemblyState {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0,
      .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
      .primitiveRestartEnable = VK_FALSE,
    };

    VkPipelineRasterizationStateCreateInfo rasterizationState {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0,
      .depthClampEnable = VK_FALSE,
      .rasterizerDiscardEnable = VK_FALSE,
      .polygonMode = VK_POLYGON_MODE_FILL,
      .cullMode = VK_CULL_MODE_BACK_BIT,
      .frontFace = VK_FRONT_FACE_CLOCKWISE,
      .depthBiasEnable = VK_FALSE,
      .depthBiasConstantFactor = 0.0f,
      .depthBiasClamp = 0.0f,
      .depthBiasSlopeFactor = 0.0f,
      .lineWidth = 1.0f,
    };

    VkPipelineMultisampleStateCreateInfo multisampleState {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0,
      .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
      .sampleShadingEnable = VK_FALSE,
      .minSampleShading = 1.0f,
      .pSampleMask = nullptr,
      .alphaToCoverageEnable = VK_FALSE,
      .alphaToOneEnable = VK_FALSE,
    };

    VkPipelineColorBlendAttachmentState colorBlendAttachment {
      .blendEnable = VK_FALSE,
      .srcColorBlendFactor = VK_BLEND_FACTOR_ONE,
      .dstColorBlendFactor = VK_BLEND_FACTOR_ZERO,
      .colorBlendOp = VK_BLEND_OP_ADD,
      .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
      .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
      .alphaBlendOp = VK_BLEND_OP_ADD,
      .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT
                        | VK_COLOR_COMPONENT_A_BIT,
    };

    VkPipelineColorBlendStateCreateInfo colorBlendState {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0,
      .logicOpEnable = VK_FALSE,
      .logicOp = VK_LOGIC_OP_COPY,
      .attachmentCount = 1,
      .pAttachments = &colorBlendAttachment,
      .blendConstants = {0.0f, 0.0f, 0.0f, 0.0f},
    };

    VkPipelineLayoutCreateInfo layoutInfo {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0,
      .setLayoutCount = 1,
      .pSetLayouts = &_descriptorSetLayout,
      .pushConstantRangeCount = 0,
      .pPushConstantRanges = nullptr,
    };
    vkCreatePipelineLayout(_device, &layoutInfo, nullptr, &_pipelineLayout);

    VkGraphicsPipelineCreateInfo pipelineInfo {
      .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0,
      .stageCount = 2,
      .pStages = shaderStages,
      .pVertexInputState = &vertexInputState,
      .pInputAssemblyState = &inputAssemblyState,
      .pTessellationState = nullptr,
      .pViewportState = &viewportState,
      .pRasterizationState = &rasterizationState,
      .pMultisampleState = &multisampleState,
      .pDepthStencilState = nullptr,
      .pColorBlendState = &colorBlendState,
      .pDynamicState = &dynamicState,
      .layout = _pipelineLayout,
      .renderPass = _renderPass,
      .subpass = 0,
      .basePipelineHandle = VK_NULL_HANDLE,
      .basePipelineIndex = -1,
    };

    if (vkCreateGraphicsPipelines(_device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &_pipeline)
        != VK_SUCCESS)
    {
      throw std::runtime_error("Failed to create graphics pipeline");
    }

    vkDestroyShaderModule(_device, vertexShaderModule, nullptr);
    vkDestroyShaderModule(_device, fragmentShaderModule, nullptr);
  }

  void VulkanContext::CreateFramebuffers()
  {
    _swapchainFramebuffers.resize(_swapchainImageViews.size());
    for (size_t i = 0; i < _swapchainImageViews.size(); i++)
    {
      VkImageView attachments[] = {_swapchainImageViews[i]};

      VkFramebufferCreateInfo framebufferInfo {};
      framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
      framebufferInfo.renderPass = _renderPass;
      framebufferInfo.attachmentCount = 1;
      framebufferInfo.pAttachments = attachments;
      framebufferInfo.width = _swapchainExtent.width;
      framebufferInfo.height = _swapchainExtent.height;
      framebufferInfo.layers = 1;

      if (vkCreateFramebuffer(_device, &framebufferInfo, nullptr, &_swapchainFramebuffers[i]) != VK_SUCCESS)
      {
        throw std::runtime_error("failed to create framebuffer!");
      }
    }
  }

  void VulkanContext::CreateCommandPool() noexcept
  {
    VkCommandPoolCreateInfo poolInfo {};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = _queueFamilyIndices.GraphicsFamily;

    if (vkCreateCommandPool(_device, &poolInfo, nullptr, &_commandPool) != VK_SUCCESS)
    {
    }
  }

  void VulkanContext::CreateCommandBuffers() noexcept
  {
    _commandBuffers.resize(MAX_FRAMES_IN_FLIGHT);
    VkCommandBufferAllocateInfo allocInfo {
      .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
      .pNext = nullptr,
      .commandPool = _commandPool,
      .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
      .commandBufferCount = static_cast<uint32>(_commandBuffers.size()),
    };

    if (vkAllocateCommandBuffers(_device, &allocInfo, _commandBuffers.data()) != VK_SUCCESS)
    {
    }
  }

  void VulkanContext::CreateSyncObjects()
  {
    _imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    _renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    _inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

    VkSemaphoreCreateInfo semaphoreInfo {
      .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0,
    };

    VkFenceCreateInfo fenceInfo {
      .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
      .pNext = nullptr,
      .flags =
        VK_FENCE_CREATE_SIGNALED_BIT, // Start with the fence signaled so we can immediately acquire an image
    };

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
      if (vkCreateSemaphore(_device, &semaphoreInfo, nullptr, &_imageAvailableSemaphores[i]) != VK_SUCCESS
          || vkCreateSemaphore(_device, &semaphoreInfo, nullptr, &_renderFinishedSemaphores[i]) != VK_SUCCESS
          || vkCreateFence(_device, &fenceInfo, nullptr, &_inFlightFences[i]) != VK_SUCCESS)
      {
        throw std::runtime_error("failed to create synchronization objects for a frame!");
      }
    }
  }

  void VulkanContext::RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32 imageIndex) const
  {
    VkCommandBufferBeginInfo beginInfo {
      .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
      .pNext = nullptr,
      .flags = 0,
      .pInheritanceInfo = nullptr,
    };

    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
    {
      throw std::runtime_error("failed to begin recording command buffer!");
    }

    VkClearValue clearColor = {.color = {0.0f, 0.0f, 0.0f, 1.0f}};
    VkRenderPassBeginInfo renderPassInfo {
      .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
      .pNext = nullptr,
      .renderPass = _renderPass,
      .framebuffer = _swapchainFramebuffers[imageIndex],
      .renderArea = {.offset = {0, 0}, .extent = _swapchainExtent},
      .clearValueCount = 1,
      .pClearValues = &clearColor,
    };

    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _pipeline);

    VkViewport viewport {
      .x = 0.0f,
      .y = 0.0f,
      .width = static_cast<float>(_swapchainExtent.width),
      .height = static_cast<float>(_swapchainExtent.height),
      .minDepth = 0.0f,
      .maxDepth = 1.0f,
    };
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

    VkRect2D scissor {
      .offset = {0, 0},
      .extent = _swapchainExtent,
    };
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

    VkBuffer vertexBuffers[] = {_vertexBuffer};
    VkDeviceSize offsets[] = {0};

    vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
    vkCmdBindIndexBuffer(commandBuffer, _indexBuffer, 0, VK_INDEX_TYPE_UINT16);
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _pipelineLayout, 0, 1,
                            &_descriptorSets[_currentFrame], 0, nullptr);

    vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);

    vkCmdEndRenderPass(commandBuffer);
    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
    {
      throw std::runtime_error("failed to record command buffer!");
    }
  }

  void VulkanContext::CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage,
                                   VkMemoryPropertyFlags properties, VkBuffer &buffer,
                                   VkDeviceMemory &bufferMemory) const
  {
    VkBufferCreateInfo bufferInfo {
      .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0,
      .size = size,
      .usage = usage,
      .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
    };

    if (vkCreateBuffer(_device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS)
    {
      throw std::runtime_error("Failed to create buffer");
    }

    VkMemoryRequirements memoryRequirements;
    vkGetBufferMemoryRequirements(_device, buffer, &memoryRequirements);

    VkMemoryAllocateInfo allocInfo {
      .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
      .pNext = nullptr,
      .allocationSize = memoryRequirements.size,
      .memoryTypeIndex = FindMemoryType(memoryRequirements.memoryTypeBits, properties),
    };

    if (vkAllocateMemory(_device, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS)
    {
      throw std::runtime_error("Failed to allocate buffer memory");
    }

    vkBindBufferMemory(_device, buffer, bufferMemory, 0);
  }

  void VulkanContext::CopyBuffer(VkBuffer src, VkBuffer dst, VkDeviceSize size) const
  {
    VkCommandBufferAllocateInfo allocInfo {
      .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
      .pNext = nullptr,
      .commandPool = _commandPool,
      .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
      .commandBufferCount = 1,
    };

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(_device, &allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo {
      .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
      .pNext = nullptr,
      .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
      .pInheritanceInfo = nullptr,
    };
    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    VkBufferCopy copyRegion {
      .srcOffset = 0,
      .dstOffset = 0,
      .size = size,
    };

    vkCmdCopyBuffer(commandBuffer, src, dst, 1, &copyRegion);

    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
    {
      throw std::runtime_error("Failed to record command buffer for buffer copy");
    }

    VkSubmitInfo submitInfo {
      .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
      .pNext = nullptr,
      .waitSemaphoreCount = 0,
      .pWaitSemaphores = nullptr,
      .pWaitDstStageMask = nullptr,
      .commandBufferCount = 1,
      .pCommandBuffers = &commandBuffer,
      .signalSemaphoreCount = 0,
      .pSignalSemaphores = nullptr,
    };

    if (vkQueueSubmit(_graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE) != VK_SUCCESS)
    {
      throw std::runtime_error("Failed to submit command buffer for buffer copy");
    }

    vkQueueWaitIdle(_graphicsQueue);
    vkFreeCommandBuffers(_device, _commandPool, 1, &commandBuffer);
  }

  void VulkanContext::CreateVertexBuffer()
  {
    VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer,
                 stagingBufferMemory);

    void *data;
    vkMapMemory(_device, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, vertices.data(), (size_t)bufferSize);
    vkUnmapMemory(_device, stagingBufferMemory);

    CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, _vertexBuffer, _vertexBufferMemory);

    CopyBuffer(stagingBuffer, _vertexBuffer, bufferSize);
    vkDestroyBuffer(_device, stagingBuffer, nullptr);
    vkFreeMemory(_device, stagingBufferMemory, nullptr);
  }

  void VulkanContext::CreateIndexBuffer()
  {
    VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer,
                 stagingBufferMemory);
    void *data;
    vkMapMemory(_device, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, indices.data(), (size_t)bufferSize);
    vkUnmapMemory(_device, stagingBufferMemory);
    CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, _indexBuffer, _indexBufferMemory);
    CopyBuffer(stagingBuffer, _indexBuffer, bufferSize);
    vkDestroyBuffer(_device, stagingBuffer, nullptr);
    vkFreeMemory(_device, stagingBufferMemory, nullptr);
  }

  void VulkanContext::CreateUniformBuffers()
  {
    VkDeviceSize bufferSize = sizeof(UniformBufferObject);

    _uniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);
    _uniformBuffersMemory.resize(MAX_FRAMES_IN_FLIGHT);
    _uniformBuffersMapped.resize(MAX_FRAMES_IN_FLIGHT);

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
      CreateBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                   VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                   _uniformBuffers[i], _uniformBuffersMemory[i]);

      vkMapMemory(_device, _uniformBuffersMemory[i], 0, bufferSize, 0, &_uniformBuffersMapped[i]);
    }
  }

  void VulkanContext::UpdateUniformBuffer(uint32 currentImage) const
  {
    static auto startTime = std::chrono::high_resolution_clock::now();

    auto currentTime = std::chrono::high_resolution_clock::now();
    float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

    using namespace Krys::Maths;

    UniformBufferObject ubo {
      .model = Rotate(Identity<Mat4>(), time * Radians(90.0f), Vec3(0.0f, 0.0f, 1.0f)),
      .view = LookAt(Vec3(2.0f, 2.0f, 2.0f), Vec3(0.0f, 0.0f, 0.0f), Vec3(0.0f, 0.0f, 1.0f)),
      .proj =
        Perspective(Radians(45.0f), _swapchainExtent.width / (float)_swapchainExtent.height, 0.1f, 10.0f)};

    ubo.proj[1][1] *= -1;

    memcpy(_uniformBuffersMapped[currentImage], &ubo, sizeof(ubo));
  }

  void VulkanContext::CreateDescriptorPool()
  {
    VkDescriptorPoolSize poolSize {
      .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
      .descriptorCount = static_cast<uint32>(MAX_FRAMES_IN_FLIGHT),
    };

    VkDescriptorPoolCreateInfo poolInfo {
      .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0,
      .maxSets = static_cast<uint32>(MAX_FRAMES_IN_FLIGHT),
      .poolSizeCount = 1,
      .pPoolSizes = &poolSize,
    };

    if (vkCreateDescriptorPool(_device, &poolInfo, nullptr, &_descriptorPool) != VK_SUCCESS)
    {
      throw std::runtime_error("Failed to create descriptor pool");
    }
  }

  void VulkanContext::CreateDescriptorSets()
  {
    List<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, _descriptorSetLayout);
    VkDescriptorSetAllocateInfo allocInfo {
      .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
      .pNext = nullptr,
      .descriptorPool = _descriptorPool,
      .descriptorSetCount = static_cast<uint32>(MAX_FRAMES_IN_FLIGHT),
      .pSetLayouts = layouts.data(),
    };

    _descriptorSets.resize(MAX_FRAMES_IN_FLIGHT);
    if (vkAllocateDescriptorSets(_device, &allocInfo, _descriptorSets.data()) != VK_SUCCESS)
    {
      throw std::runtime_error("Failed to allocate descriptor sets");
    }

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
      VkDescriptorBufferInfo bufferInfo {
        .buffer = _uniformBuffers[i],
        .offset = 0,
        .range = sizeof(UniformBufferObject),
      };

      VkWriteDescriptorSet descriptorWrite {
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .pNext = nullptr,
        .dstSet = _descriptorSets[i],
        .dstBinding = 0,
        .dstArrayElement = 0,
        .descriptorCount = 1,
        .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        .pImageInfo = nullptr,
        .pBufferInfo = &bufferInfo,
        .pTexelBufferView = nullptr,
      };
      vkUpdateDescriptorSets(_device, 1, &descriptorWrite, 0, nullptr);
    }
  }

  uint32 VulkanContext::FindMemoryType(uint32 typeFilter, VkMemoryPropertyFlags properties) const
  {
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(_physicalDevice, &memProperties);
    for (uint32 i = 0; i < memProperties.memoryTypeCount; i++)
    {
      if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
      {
        return i;
      }
    }
    throw std::runtime_error("Failed to find suitable memory type");
  }

  void VulkanContext::Setup() noexcept
  {
  }

  void VulkanContext::Render(ICamera &camera) noexcept
  {
    vkWaitForFences(_device, 1, &_inFlightFences[_currentFrame], VK_TRUE, UINT64_MAX);

    VkResult result =
      vkAcquireNextImageKHR(_device, _swapchain, UINT64_MAX, _imageAvailableSemaphores[_currentFrame],
                            VK_NULL_HANDLE, &_currentImageIndex);
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
    {
      RecreateSwapchain();
      return;
    }
    else if (result != VK_SUCCESS)
    {
      /*throw std::runtime_error("Failed to acquire swap chain image");*/
    }

    UpdateUniformBuffer(_currentFrame);

    vkResetFences(_device, 1, &_inFlightFences[_currentFrame]);

    vkResetCommandBuffer(_commandBuffers[_currentFrame], 0);
    RecordCommandBuffer(_commandBuffers[_currentFrame], _currentImageIndex);

    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    VkSubmitInfo submitInfo {
      .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
      .pNext = nullptr,
      .waitSemaphoreCount = 1,
      .pWaitSemaphores = &_imageAvailableSemaphores[_currentFrame],
      .pWaitDstStageMask = waitStages,
      .commandBufferCount = 1,
      .pCommandBuffers = &_commandBuffers[_currentFrame],
      .signalSemaphoreCount = 1,
      .pSignalSemaphores = &_renderFinishedSemaphores[_currentFrame],
    };

    vkQueueSubmit(_graphicsQueue, 1, &submitInfo, _inFlightFences[_currentFrame]);

    VkPresentInfoKHR presentInfo {
      .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
      .pNext = nullptr,
      .waitSemaphoreCount = 1,
      .pWaitSemaphores = &_renderFinishedSemaphores[_currentFrame],
      .swapchainCount = 1,
      .pSwapchains = &_swapchain,
      .pImageIndices = &_currentImageIndex,
      .pResults = nullptr,
    };

    result = vkQueuePresentKHR(_presentQueue, &presentInfo);
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || _framebufferResized)
    {
      RecreateSwapchain();
      return;
    }
    _currentFrame = (_currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
  }

  void VulkanContext::Present() noexcept
  {
  }

  void VulkanContext::Resize(uint32, uint32) noexcept
  {
    _framebufferResized = true;
  }
}