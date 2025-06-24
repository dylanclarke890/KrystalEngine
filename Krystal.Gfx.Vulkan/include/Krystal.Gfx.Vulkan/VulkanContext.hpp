#pragma once

#include "Krystal.Core/Core.hpp"
#include "Krystal.Gfx/IContext.hpp"

#include "Krystal.Gfx.Vulkan/Hooks/vulkan_hooks.hpp"

namespace Krys::Gfx::Vulkan
{
  struct QueueFamilyIndices
  {
    Krys::uint32 GraphicsFamily {};
    Krys::uint32 PresentFamily {};
  };

  struct SwapchainSupportDetails
  {
    VkSurfaceCapabilitiesKHR SurfaceCapabilities {};
    List<VkSurfaceFormatKHR> Formats {};
    List<VkPresentModeKHR> PresentModes {};
  };

  class VulkanContext : public IContext
  {
    NativeHandle _windowHandle = nullptr;
    uint32 _width = 0, _height = 0;

    VkInstance _instance {VK_NULL_HANDLE};
    VkSurfaceKHR _surface = VK_NULL_HANDLE;
    VkDebugUtilsMessengerEXT _debugMessenger {VK_NULL_HANDLE};
    QueueFamilyIndices _queueFamilyIndices {};
    VkPhysicalDevice _physicalDevice {VK_NULL_HANDLE};

    VkDevice _device = VK_NULL_HANDLE;
    VkQueue _presentQueue = VK_NULL_HANDLE;

    VkSwapchainKHR _swapchain = VK_NULL_HANDLE;
    List<VkImage> _swapchainImages {};
    VkFormat _swapchainImageFormat;
    VkExtent2D _swapchainExtent;
    List<VkImageView> _swapchainImageViews {};
    List<VkFramebuffer> _swapchainFramebuffers {};

    VkRenderPass _renderPass = VK_NULL_HANDLE;
    VkPipelineLayout _pipelineLayout = VK_NULL_HANDLE;
    VkPipeline _pipeline = VK_NULL_HANDLE;

    VkCommandPool _commandPool = VK_NULL_HANDLE;
    List<VkCommandBuffer> _commandBuffers {};

    List<VkSemaphore> _imageAvailableSemaphores {};
    List<VkSemaphore> _renderFinishedSemaphores {};
    List<VkFence> _inFlightFences {};

    uint32 _currentImageIndex = 0;
    uint32 _currentFrame = 0;
    bool _framebufferResized = false;

    NO_COPY_MOVE(VulkanContext)
  public:
    VulkanContext(NativeHandle windowHandle, uint32 width, uint32 height);

    ~VulkanContext() noexcept;

    void SetupTestTriangle() noexcept override;

    void DrawTestTriangle() noexcept override;

    void Present() noexcept override;

    virtual void Resize(uint32 width, uint32 height) noexcept override;

  private:
    void CreateInstance();

    void SetupDebugMessenger();
    bool AreValidationLayersAvailable();

    void SelectPhysicalDevice();
    bool IsDeviceSuitable(VkPhysicalDevice device);
    Nullable<QueueFamilyIndices> FindQueueFamilyIndices(VkPhysicalDevice device) const;
    bool AreDeviceExtensionsSupported(VkPhysicalDevice device) const noexcept;
    SwapchainSupportDetails QuerySwapchainSupport(VkPhysicalDevice device) const noexcept;

    void CreateDevice();

    void CreateSwapchain();
    VkSurfaceFormatKHR
      ChooseSwapSurfaceFormat(const List<VkSurfaceFormatKHR> &availableFormats) const noexcept;
    VkPresentModeKHR
      ChooseSwapPresentMode(const List<VkPresentModeKHR> &availablePresentModes) const noexcept;
    VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities) const noexcept;
    void CreateSwapchainImageViews();
    void RecreateSwapchain();
    void CleanupSwapchain();

    void CreateRenderPass() noexcept;
    void CreateGraphicsPipeline();
    void CreateFramebuffers();
    void CreateCommandPool() noexcept;
    void CreateCommandBuffers() noexcept;
    void CreateSyncObjects();

    void RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32 imageIndex) const;
  };
}
