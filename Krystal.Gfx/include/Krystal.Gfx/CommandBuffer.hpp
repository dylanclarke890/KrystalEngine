#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Commands/CommandList.hpp"
#include "Krystal.Core/Concepts.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/Types/String.hpp"
#include "Krystal.Gfx/Commands.hpp"
#include "Krystal.Gfx/RenderPass.hpp"
#include <cstring>

namespace krys::gfx
{
  /// @brief Records commands as plain data for a device to replay on `Device::Submit`. Recording costs a copy
  /// per command. Misuse (drawing outside a render pass or without a pipeline, nesting render passes) asserts
  /// where it happens.
  class CommandBuffer
  {
    KRYS_NON_COPY_MOVABLE(CommandBuffer);

  private:
    CommandList _commands;
    bool _inRenderPass {false};
    bool _hasPipeline {false};
    uint32 _debugGroupDepth {0u};

  public:
    CommandBuffer() noexcept = default;
    ~CommandBuffer() noexcept = default;

    void BeginRenderPass(const RenderPassDesc &desc) noexcept;
    void EndRenderPass() noexcept;

    /// @brief Sets the viewport. Coordinates have a top-left origin, y pointing down, in pixels. A render
    /// pass starts with the viewport covering its targets.
    void SetViewport(float x, float y, float width, float height, float minDepth = 0.f,
                     float maxDepth = 1.f) noexcept;
    /// @brief Sets the scissor rectangle, used by pipelines with `RasterState::ScissorTest`. Coordinates have
    /// a top-left origin, y pointing down, in pixels. A render pass starts with the scissor covering its
    /// targets.
    void SetScissor(int32 x, int32 y, uint32 width, uint32 height) noexcept;
    void SetBlendConstant(float red, float green, float blue, float alpha) noexcept;
    void SetStencilReference(uint32 reference) noexcept;

    void BindPipeline(PipelineHandle pipeline) noexcept;
    void BindVertexBuffer(uint32 slot, BufferHandle buffer, uint64 offset = 0u) noexcept;
    void BindIndexBuffer(BufferHandle buffer, IndexType type, uint64 offset = 0u) noexcept;
    void BindUniformBuffer(uint32 slot, BufferHandle buffer, uint64 offset, uint64 size) noexcept;
    void BindTexture(uint32 slot, TextureHandle texture, SamplerHandle sampler) noexcept;

    /// @brief Sets the bound pipeline's push constants. Stays set until the next call.
    template <HasStandardLayout T>
    void PushConstants(const T &data) noexcept
    {
      static_assert(sizeof(T) <= MaxPushConstantSize, "Push constants are limited to 128 bytes.");
      PushConstantBytes(&data, sizeof(T));
    }

    void PushConstantBytes(const void *data, uint32 size) noexcept;

    void Draw(uint32 vertexCount, uint32 instanceCount = 1u, uint32 firstVertex = 0u,
              uint32 firstInstance = 0u) noexcept;
    void DrawIndexed(uint32 indexCount, uint32 instanceCount = 1u, uint32 firstIndex = 0u,
                     int32 vertexOffset = 0, uint32 firstInstance = 0u) noexcept;

    /// @brief Copies tightly packed texels from a buffer into a texture region. Outside render passes.
    void CopyBufferToTexture(BufferHandle source, uint64 sourceOffset, TextureHandle destination,
                             const TextureRegion &region) noexcept;
    /// @brief Copies a 2D region of one texture into another, scaling with the filter if the sizes differ.
    /// Outside render passes.
    void BlitTexture(TextureHandle source, const TextureRegion &sourceRegion, TextureHandle destination,
                     const TextureRegion &destinationRegion,
                     SamplerFilter filter = SamplerFilter::Linear) noexcept;

    /// @brief Starts a named group of commands, shown in graphics debuggers.
    void PushDebugGroup(utf8_stringview name) noexcept;
    void PopDebugGroup() noexcept;

    /// @brief Discards every recorded command.
    void Reset() noexcept;

    KRYS_NODISCARD const CommandList &Commands() const noexcept
    {
      return _commands;
    }

    KRYS_NODISCARD bool IsEmpty() const noexcept
    {
      return _commands.SizeInBytes() == 0u;
    }

    /// @brief Whether the recording is complete: no render pass or debug group left open.
    KRYS_NODISCARD bool IsComplete() const noexcept
    {
      return !_inRenderPass && _debugGroupDepth == 0u;
    }
  };
}
