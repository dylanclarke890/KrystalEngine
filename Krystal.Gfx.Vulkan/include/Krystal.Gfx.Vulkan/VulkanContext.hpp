#pragma once

#include "Krystal.Gfx/IContext.hpp"
#include "Krystal.IO/Image.hpp"
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
    VkQueue _graphicsQueue = VK_NULL_HANDLE;

    VkSwapchainKHR _swapchain = VK_NULL_HANDLE;
    List<VkImage> _swapchainImages {};
    VkFormat _swapchainImageFormat;
    VkExtent2D _swapchainExtent;
    List<VkImageView> _swapchainImageViews {};
    List<VkFramebuffer> _swapchainFramebuffers {};

    VkRenderPass _renderPass = VK_NULL_HANDLE;
    VkDescriptorSetLayout _descriptorSetLayout = VK_NULL_HANDLE;
    VkPipelineLayout _pipelineLayout = VK_NULL_HANDLE;
    VkPipeline _pipeline = VK_NULL_HANDLE;
    VkBuffer _vertexBuffer = VK_NULL_HANDLE;
    VkBuffer _indexBuffer = VK_NULL_HANDLE;
    VkDeviceMemory _vertexBufferMemory = VK_NULL_HANDLE;
    VkDeviceMemory _indexBufferMemory = VK_NULL_HANDLE;

    List<VkBuffer> _uniformBuffers;
    List<VkDeviceMemory> _uniformBuffersMemory;
    List<void *> _uniformBuffersMapped;

    VkDescriptorPool _descriptorPool = VK_NULL_HANDLE;
    List<VkDescriptorSet> _descriptorSets;

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

    void Setup() noexcept override;

    void Render(ICamera &camera) noexcept override;

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
    void CreateDescriptorSetLayout();
    void CreateGraphicsPipeline();
    void CreateFramebuffers();
    void CreateCommandPool() noexcept;
    void CreateCommandBuffers() noexcept;
    void CreateSyncObjects();
    void CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties,
                      VkBuffer &buffer, VkDeviceMemory &bufferMemory) const;
    void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) const;
    void CreateVertexBuffer();
    void CreateIndexBuffer();
    void CreateUniformBuffers();
    void UpdateUniformBuffer(uint32 currentImageIndex) const;
    uint32 FindMemoryType(uint32 typeFilter, VkMemoryPropertyFlags properties) const;

    void CreateDescriptorPool();
    void CreateDescriptorSets();

    void RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32 imageIndex) const;
  };
}
