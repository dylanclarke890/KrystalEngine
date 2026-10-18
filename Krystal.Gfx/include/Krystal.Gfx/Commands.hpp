#pragma once

#include "Krystal.Core/Commands/CommandType.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/Types/Array.hpp"
#include "Krystal.Gfx/Formats.hpp"
#include "Krystal.Gfx/Handles.hpp"
#include "Krystal.Gfx/Pipeline.hpp"
#include "Krystal.Gfx/RenderPass.hpp"
#include "Krystal.Gfx/Sampler.hpp"
#include "Krystal.Gfx/Texture.hpp"

/// @brief The commands a `CommandBuffer` records, as plain data for a device to replay.
namespace krys::gfx::commands
{
  struct BeginRenderPass
  {
    constexpr static CommandType Type = KRYS_CMD_TYPE("gfx::BeginRenderPass");
    Array<ColourAttachment, MaxColourAttachments> ColourAttachments;
    uint32 ColourAttachmentCount;
    DepthStencilAttachment DepthStencil;
  };

  struct EndRenderPass
  {
    constexpr static CommandType Type = KRYS_CMD_TYPE("gfx::EndRenderPass");
  };

  /// @brief Coordinates have a top-left origin, y pointing down, in pixels.
  struct SetViewport
  {
    constexpr static CommandType Type = KRYS_CMD_TYPE("gfx::SetViewport");
    float X;
    float Y;
    float Width;
    float Height;
    float MinDepth;
    float MaxDepth;
  };

  /// @brief Coordinates have a top-left origin, y pointing down, in pixels.
  struct SetScissor
  {
    constexpr static CommandType Type = KRYS_CMD_TYPE("gfx::SetScissor");
    int32 X;
    int32 Y;
    uint32 Width;
    uint32 Height;
  };

  struct SetBlendConstant
  {
    constexpr static CommandType Type = KRYS_CMD_TYPE("gfx::SetBlendConstant");
    Array<float, 4> Constant;
  };

  struct SetStencilReference
  {
    constexpr static CommandType Type = KRYS_CMD_TYPE("gfx::SetStencilReference");
    uint32 Reference;
  };

  struct BindPipeline
  {
    constexpr static CommandType Type = KRYS_CMD_TYPE("gfx::BindPipeline");
    PipelineHandle Pipeline;
  };

  struct BindVertexBuffer
  {
    constexpr static CommandType Type = KRYS_CMD_TYPE("gfx::BindVertexBuffer");
    uint32 Slot;
    BufferHandle Buffer;
    uint64 Offset;
  };

  struct BindIndexBuffer
  {
    constexpr static CommandType Type = KRYS_CMD_TYPE("gfx::BindIndexBuffer");
    BufferHandle Buffer;
    gfx::IndexType IndexType;
    uint64 Offset;
  };

  struct BindUniformBuffer
  {
    constexpr static CommandType Type = KRYS_CMD_TYPE("gfx::BindUniformBuffer");
    uint32 Slot;
    BufferHandle Buffer;
    uint64 Offset;
    uint64 Size;
  };

  struct BindTexture
  {
    constexpr static CommandType Type = KRYS_CMD_TYPE("gfx::BindTexture");
    uint32 Slot;
    TextureHandle Texture;
    SamplerHandle Sampler;
  };

  struct PushConstants
  {
    constexpr static CommandType Type = KRYS_CMD_TYPE("gfx::PushConstants");
    uint32 Size;
    Array<byte, MaxPushConstantSize> Data;
  };

  struct Draw
  {
    constexpr static CommandType Type = KRYS_CMD_TYPE("gfx::Draw");
    uint32 VertexCount;
    uint32 InstanceCount;
    uint32 FirstVertex;
    uint32 FirstInstance;
  };

  struct DrawIndexed
  {
    constexpr static CommandType Type = KRYS_CMD_TYPE("gfx::DrawIndexed");
    uint32 IndexCount;
    uint32 InstanceCount;
    uint32 FirstIndex;
    int32 VertexOffset;
    uint32 FirstInstance;
  };

  /// @brief Copies tightly packed texels from a buffer into a texture region.
  struct CopyBufferToTexture
  {
    constexpr static CommandType Type = KRYS_CMD_TYPE("gfx::CopyBufferToTexture");
    BufferHandle Source;
    uint64 SourceOffset;
    TextureHandle Destination;
    TextureRegion Region;
  };

  /// @brief Copies a region of one texture into a region of another, scaling with the filter if the sizes
  /// differ. Both regions are 2D, in a single mip level and layer.
  struct BlitTexture
  {
    constexpr static CommandType Type = KRYS_CMD_TYPE("gfx::BlitTexture");
    TextureHandle Source;
    TextureRegion SourceRegion;
    TextureHandle Destination;
    TextureRegion DestinationRegion;
    SamplerFilter Filter;
  };

  struct PushDebugGroup
  {
    constexpr static CommandType Type = KRYS_CMD_TYPE("gfx::PushDebugGroup");
    /// @brief Null-terminated, truncated to fit.
    Array<char, 64> Name;
  };

  struct PopDebugGroup
  {
    constexpr static CommandType Type = KRYS_CMD_TYPE("gfx::PopDebugGroup");
  };
}
