#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/Types/Array.hpp"
#include "Krystal.Core/Types/String.hpp"
#include "Krystal.Gfx/Handles.hpp"

namespace krys::gfx
{
  /// @brief The most colour targets a render pass can have.
  constexpr uint32 MaxColourAttachments = 8u;

  /// @brief What happens to a target's contents when a render pass begins.
  enum class AttachmentLoadOp : uint8
  {
    /// @brief Keep the contents.
    Load,
    /// @brief Clear to the attachment's clear value.
    Clear,
    /// @brief The contents are undefined; every pixel will be overwritten.
    DontCare,
  };

  /// @brief What happens to a target's contents when a render pass ends.
  enum class AttachmentStoreOp : uint8
  {
    Store,
    /// @brief The contents may be discarded.
    DontCare,
  };

  /// @brief A colour target: a texture (with `TextureUsage::RenderTarget`) or a swapchain.
  struct ColourAttachment
  {
    TextureHandle Texture {};
    /// @brief Used instead of `Texture` when valid.
    SwapchainHandle Swapchain {};
    uint32 MipLevel {0u};
    /// @brief The array layer, or cube face.
    uint32 Layer {0u};
    AttachmentLoadOp Load {AttachmentLoadOp::Clear};
    AttachmentStoreOp Store {AttachmentStoreOp::Store};
    /// @brief The clear value, in the target's encoding: linear for sRGB targets, which encode on write.
    Array<float, 4> ClearColour {0.f, 0.f, 0.f, 1.f};
  };

  struct DepthStencilAttachment
  {
    TextureHandle Texture {};
    AttachmentLoadOp DepthLoad {AttachmentLoadOp::Clear};
    AttachmentStoreOp DepthStore {AttachmentStoreOp::DontCare};
    AttachmentLoadOp StencilLoad {AttachmentLoadOp::DontCare};
    AttachmentStoreOp StencilStore {AttachmentStoreOp::DontCare};
    float ClearDepth {1.f};
    uint8 ClearStencil {0u};
  };

  /// @brief The targets drawn into between `BeginRenderPass` and `EndRenderPass`.
  struct RenderPassDesc
  {
    Array<ColourAttachment, MaxColourAttachments> ColourAttachments {};
    uint32 ColourAttachmentCount {0u};
    /// @brief Unused if its texture is invalid.
    DepthStencilAttachment DepthStencil {};
    utf8_stringview DebugName {};

    /// @brief A pass with a single colour target.
    KRYS_NODISCARD constexpr static RenderPassDesc SingleColour(const ColourAttachment &attachment) noexcept
    {
      RenderPassDesc desc {};
      desc.ColourAttachments[0] = attachment;
      desc.ColourAttachmentCount = 1u;
      return desc;
    }
  };
}
