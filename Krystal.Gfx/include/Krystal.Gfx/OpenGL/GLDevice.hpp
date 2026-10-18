#pragma once

#include "Krystal.Core/Types/Array.hpp"
#include "Krystal.Core/Types/HashMap.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/String.hpp"
#include "Krystal.Gfx/Device.hpp"
#include "Krystal.Gfx/OpenGL/gl.hpp"
#include "Krystal.Gfx/OpenGL/GLContext.hpp"
#include "Krystal.Gfx/Pool.hpp"

namespace krys::gfx::opengl
{
  struct GLBuffer
  {
    GLuint Id {0u};
    uint64 Size {0u};
    /// @brief Whether the device owns the buffer (the transient ring), so it can't be destroyed through the
    /// API.
    bool Internal {false};
    string DebugName {};
  };

  struct GLTexture
  {
    GLuint Id {0u};
    GLenum Target {GL_TEXTURE_2D};
    TextureDesc Desc {};
    string DebugName {};
  };

  struct GLSampler
  {
    GLuint Id {0u};
    SamplerDesc Desc {};
    uint32 References {0u};
  };

  struct GLShader
  {
    GLuint Id {0u};
    ShaderStage Stage {ShaderStage::Vertex};
    string DebugName {};
  };

  /// @brief A vertex or index buffer binding as last applied to a pipeline's vertex array.
  struct GLBufferBinding
  {
    GLuint Buffer {0u};
    uint64 Offset {0u};

    constexpr bool operator==(const GLBufferBinding &) const noexcept = default;
  };

  struct GLPipeline
  {
    GLuint Program {0u};
    GLuint VertexArray {0u};
    GLenum Topology {GL_TRIANGLES};
    SmallList<uint32, 2> Strides {};
    BlendState Blend {};
    DepthStencilState DepthStencil {};
    RasterState Raster {};
    uint32 PushConstantSize {0u};
    /// @brief The buffers the vertex array was last given, per slot; buffers are rebound only when they
    /// change.
    SmallList<GLBufferBinding, 2> AppliedVertexBuffers {};
    GLuint AppliedIndexBuffer {0u};
    string DebugName {};
  };

  struct GLSwapchain
  {
    NativeHandle Window {};
    NativeHandle Surface {};
    uint32 Width {0u};
    uint32 Height {0u};
    TextureFormat Format {TextureFormat::RGBA8Srgb};
    bool VSync {true};
  };

  /// @brief The fixed-function state last applied, so binding a pipeline only changes what differs.
  struct GLStateCache
  {
    bool Valid {false};
    BlendState Blend {};
    DepthStencilState DepthStencil {};
    RasterState Raster {};
    bool FrontFaceFlipped {false};
    uint32 StencilReference {0u};
  };

  /// @brief The OpenGL 4.5 device.
  class GLDevice final : public Device
  {
    /// @brief The textures a framebuffer object was built from.
    struct FramebufferKey
    {
      Array<GLuint, MaxColourAttachments + 1u> Textures {};
      Array<uint16, MaxColourAttachments + 1u> MipLevels {};
      Array<uint16, MaxColourAttachments + 1u> Layers {};
      uint32 ColourCount {0u};

      constexpr bool operator==(const FramebufferKey &) const noexcept = default;
    };

    struct FramebufferKeyHash
    {
      KRYS_NODISCARD size_t operator()(const FramebufferKey &key) const noexcept;
    };

    /// @brief The state of a submission being replayed.
    struct ReplayState
    {
      bool InRenderPass {false};
      bool TargetIsSwapchain {false};
      uint32 TargetHeight {0u};
      GLuint Framebuffer {0u};
      GLPipeline *Pipeline {nullptr};
      Array<GLBufferBinding, 8> VertexBuffers {};
      GLuint IndexBuffer {0u};
      uint64 IndexOffset {0u};
      gfx::IndexType IndexType {gfx::IndexType::UInt32};
      Array<byte, MaxPushConstantSize> PushConstants {};
      uint32 PushConstantSize {0u};
      bool PushConstantsDirty {false};
      /// @brief The depth-stencil attachment and the colour attachments stored with `DontCare`.
      Array<GLenum, MaxColourAttachments + 2u> Discards {};
      uint32 DiscardCount {0u};
    };

  private:
    DeviceDesc _desc;
    UniquePtr<GLContext> _context;
    DeviceLimits _limits {};

    Pool<GLBuffer, BufferHandle> _buffers {};
    Pool<GLTexture, TextureHandle> _textures {};
    Pool<GLSampler, SamplerHandle> _samplers {};
    Pool<GLShader, ShaderHandle> _shaders {};
    Pool<GLPipeline, PipelineHandle> _pipelines {};
    Pool<GLSwapchain, SwapchainHandle> _swapchains {};

    HashMap<SamplerDesc, SamplerHandle, SamplerDescHash> _samplerCache {};
    HashMap<FramebufferKey, GLuint, FramebufferKeyHash> _framebuffers {};
    /// @brief Temporary framebuffers for blits.
    Array<GLuint, 2> _blitFramebuffers {0u, 0u};

    GLStateCache _state {};
    CommandBuffer _commandBuffer {};

    BufferHandle _transientBuffer {};
    byte *_transientMemory {nullptr};
    uint64 _transientOffset {0u};
    uint32 _frameIndex {0u};
    Array<GLsync, FramesInFlight> _frameFences {};
    bool _inFrame {false};

  public:
    explicit GLDevice(const DeviceDesc &desc) noexcept;
    ~GLDevice() noexcept override;

    /// @brief Creates the context and the device's own resources.
    KRYS_NODISCARD Expected<void> Initialise() noexcept;

    KRYS_NODISCARD Expected<BufferHandle> CreateBuffer(const BufferDesc &desc) noexcept override;
    KRYS_NODISCARD Expected<TextureHandle> CreateTexture(const TextureDesc &desc) noexcept override;
    KRYS_NODISCARD Expected<SamplerHandle> CreateSampler(const SamplerDesc &desc) noexcept override;
    KRYS_NODISCARD Expected<ShaderHandle> CreateShader(const ShaderDesc &desc) noexcept override;
    KRYS_NODISCARD Expected<PipelineHandle> CreatePipeline(const PipelineDesc &desc) noexcept override;
    KRYS_NODISCARD Expected<SwapchainHandle> CreateSwapchain(const SwapchainDesc &desc) noexcept override;

    void Destroy(BufferHandle buffer) noexcept override;
    void Destroy(TextureHandle texture) noexcept override;
    void Destroy(SamplerHandle sampler) noexcept override;
    void Destroy(ShaderHandle shader) noexcept override;
    void Destroy(PipelineHandle pipeline) noexcept override;
    void Destroy(SwapchainHandle swapchain) noexcept override;

    void ResizeSwapchain(SwapchainHandle swapchain, uint32 width, uint32 height) noexcept override;
    void SetVSync(SwapchainHandle swapchain, bool enabled) noexcept override;

    void WriteBuffer(BufferHandle buffer, uint64 offset, Span<const byte> data) noexcept override;
    void WriteTexture(TextureHandle texture, const TextureRegion &region,
                      Span<const byte> data) noexcept override;
    void ReadTexture(TextureHandle texture, const TextureRegion &region, Span<byte> out) noexcept override;
    void GenerateMipmaps(TextureHandle texture) noexcept override;

    KRYS_NODISCARD TransientAllocation AllocateTransient(uint64 size,
                                                         uint32 alignment = 256u) noexcept override;

    void BeginFrame() noexcept override;
    KRYS_NODISCARD CommandBuffer &GetCommandBuffer() noexcept override;
    void Submit(CommandBuffer &commands) noexcept override;
    void Present(SwapchainHandle swapchain) noexcept override;
    void EndFrame() noexcept override;
    void WaitIdle() noexcept override;

    KRYS_NODISCARD const DeviceLimits &Limits() const noexcept override;
    KRYS_NODISCARD Maybe<TextureDesc> GetTextureDesc(TextureHandle texture) const noexcept override;

  private:
    void QueryLimits() noexcept;
    KRYS_NODISCARD Expected<void> CreateTransientRing() noexcept;
    void ReportLeaks() noexcept;
    void DestroyAll() noexcept;
    void ForgetFramebuffersUsing(GLuint texture) noexcept;
    KRYS_NODISCARD GLuint GetFramebuffer(const FramebufferKey &key) noexcept;

    void ExecuteBeginRenderPass(ReplayState &replay, const commands::BeginRenderPass &command) noexcept;
    void ExecuteEndRenderPass(ReplayState &replay) noexcept;
    void ExecuteBindPipeline(ReplayState &replay, PipelineHandle handle) noexcept;
    void ApplyPipelineState(const GLPipeline &pipeline, bool flipFrontFace) noexcept;
    void PrepareDraw(ReplayState &replay) noexcept;
    void ExecuteCopyBufferToTexture(const commands::CopyBufferToTexture &command) noexcept;
    void ExecuteBlitTexture(const commands::BlitTexture &command) noexcept;
  };
}
