#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/Types/Expected.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/Span.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"
#include "Krystal.Gfx/Buffer.hpp"
#include "Krystal.Gfx/CommandBuffer.hpp"
#include "Krystal.Gfx/Handles.hpp"
#include "Krystal.Gfx/Pipeline.hpp"
#include "Krystal.Gfx/Sampler.hpp"
#include "Krystal.Gfx/Shader.hpp"
#include "Krystal.Gfx/Swapchain.hpp"
#include "Krystal.Gfx/Texture.hpp"

namespace krys::gfx
{
  struct DeviceDesc
  {
    /// @brief Enables the API's debug output, logged through the global logger.
    bool EnableValidation {false};
    /// @brief The size of each frame's transient memory, in bytes.
    uint64 TransientMemoryPerFrame {8ull * 1'024ull * 1'024ull};
  };

  struct DeviceLimits
  {
    /// @brief The alignment of a uniform buffer binding's offset.
    uint32 UniformBufferOffsetAlignment {256u};
    uint32 MaxTextureSize {0u};
    uint32 MaxColourAttachments {0u};
    float MaxSamplerAnisotropy {1.f};
  };

  /// @brief Memory from the device's per-frame ring, valid until `EndFrame`.
  struct TransientAllocation
  {
    BufferHandle Buffer {};
    uint64 Offset {0u};
    /// @brief Write here. Empty if the frame's transient memory is used up.
    Span<byte> Data {};

    KRYS_NODISCARD bool IsValid() const noexcept
    {
      return !Data.empty();
    }
  };

  /// @brief Creates and destroys GPU resources, and submits recorded commands.
  ///
  /// Whoever creates a resource owns it and destroys it. Destroying a resource that recorded but unsubmitted
  /// commands use is an error. Creation reports failures (with the compile or link log for shaders and
  /// pipelines); using a stale or invalid handle asserts in debug builds and does nothing otherwise.
  ///
  /// A frame:
  /// @code
  /// device.BeginFrame();
  /// auto &commands = device.GetCommandBuffer();
  /// commands.BeginRenderPass(...);
  /// ...
  /// commands.EndRenderPass();
  /// device.Submit(commands);
  /// device.Present(swapchain);
  /// device.EndFrame();
  /// @endcode
  class Device
  {
    KRYS_NON_COPY_MOVABLE(Device);

  protected:
    Device() noexcept = default;

  public:
    /// @brief Creates the device for the platform's graphics API (OpenGL 4.5).
    KRYS_NODISCARD static Expected<UniquePtr<Device>> Create(const DeviceDesc &desc) noexcept;

    virtual ~Device() noexcept = default;

    KRYS_NODISCARD virtual Expected<BufferHandle> CreateBuffer(const BufferDesc &desc) noexcept = 0;
    KRYS_NODISCARD virtual Expected<TextureHandle> CreateTexture(const TextureDesc &desc) noexcept = 0;
    /// @brief Returns the existing sampler for an equal description.
    KRYS_NODISCARD virtual Expected<SamplerHandle> CreateSampler(const SamplerDesc &desc) noexcept = 0;
    /// @brief Compiles a shader stage; the error is the compile log.
    KRYS_NODISCARD virtual Expected<ShaderHandle> CreateShader(const ShaderDesc &desc) noexcept = 0;
    /// @brief Links a pipeline; the error is the link log.
    KRYS_NODISCARD virtual Expected<PipelineHandle> CreatePipeline(const PipelineDesc &desc) noexcept = 0;
    KRYS_NODISCARD virtual Expected<SwapchainHandle> CreateSwapchain(const SwapchainDesc &desc) noexcept = 0;

    virtual void Destroy(BufferHandle buffer) noexcept = 0;
    virtual void Destroy(TextureHandle texture) noexcept = 0;
    /// @brief Releases one reference to a shared sampler.
    virtual void Destroy(SamplerHandle sampler) noexcept = 0;
    /// @brief Pipelines keep their own copy of the compiled stages, so shaders can be destroyed once their
    /// pipelines are created.
    virtual void Destroy(ShaderHandle shader) noexcept = 0;
    virtual void Destroy(PipelineHandle pipeline) noexcept = 0;
    virtual void Destroy(SwapchainHandle swapchain) noexcept = 0;

    virtual void ResizeSwapchain(SwapchainHandle swapchain, uint32 width, uint32 height) noexcept = 0;
    virtual void SetVSync(SwapchainHandle swapchain, bool enabled) noexcept = 0;

    /// @brief Writes bytes into a buffer at an offset.
    virtual void WriteBuffer(BufferHandle buffer, uint64 offset, Span<const byte> data) noexcept = 0;
    /// @brief Writes tightly packed texels into a texture region.
    virtual void WriteTexture(TextureHandle texture, const TextureRegion &region,
                              Span<const byte> data) noexcept = 0;
    /// @brief Reads a texture region back into tightly packed texels; waits for the GPU.
    virtual void ReadTexture(TextureHandle texture, const TextureRegion &region, Span<byte> out) noexcept = 0;
    /// @brief Fills every mip level below the base level from the base level.
    virtual void GenerateMipmaps(TextureHandle texture) noexcept = 0;

    /// @brief Allocates memory from the current frame's transient ring.
    KRYS_NODISCARD virtual TransientAllocation AllocateTransient(uint64 size,
                                                                 uint32 alignment = 256u) noexcept = 0;

    /// @brief Starts a frame: waits until the GPU has finished with the transient memory the frame reuses,
    /// and resets the command buffer.
    virtual void BeginFrame() noexcept = 0;
    /// @brief The command buffer for this frame.
    KRYS_NODISCARD virtual CommandBuffer &GetCommandBuffer() noexcept = 0;
    /// @brief Executes recorded commands, then resets the command buffer.
    virtual void Submit(CommandBuffer &commands) noexcept = 0;
    virtual void Present(SwapchainHandle swapchain) noexcept = 0;
    /// @brief Ends a frame; its transient memory is reused `FramesInFlight` frames later.
    virtual void EndFrame() noexcept = 0;
    /// @brief Waits until the GPU has finished every submitted command.
    virtual void WaitIdle() noexcept = 0;

    KRYS_NODISCARD virtual const DeviceLimits &Limits() const noexcept = 0;
    KRYS_NODISCARD virtual Maybe<TextureDesc> GetTextureDesc(TextureHandle texture) const noexcept = 0;

    /// @brief The number of frames whose transient memory can be in use at once.
    constexpr static uint32 FramesInFlight = 3u;
  };
}
