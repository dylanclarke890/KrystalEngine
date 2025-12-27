#pragma once

#include "Krystal.Gfx/Resources/Image.hpp"
#include "Krystal.Lib/Types/List.hpp"
#include "Krystal.Lib/Types/Numeric.hpp"
#include "Krystal.Lib/Types/Pair.hpp"
#include "Krystal.Lib/Types/Variant.hpp"
#include "Krystal.Maths/Vector.hpp"

namespace Krys::Gfx
{
  enum class AttachmentType
  {
    Colour,
    Depth,
    Stencil,
    DepthStencil
  };

  /// @brief Defines how an attachment should be handled when a render pass begins.
  enum class AttachmentLoadOp : uint8
  {
    Load,
    Clear,
    DontCare
  };

  /// @brief Defines how an attachment should be handled when a render pass ends.
  enum class AttachmentStoreOp : uint8
  {
    Store,
    DontCare
  };

  struct ClearColourValue
  {
    std::variant<Maths::Vec4f, Maths::Vec4i, Maths::Vec4u> Value;
  };

  enum class AttachmentClearValueType : uint8
  {
    Colour,
    Depth,
    Stencil,
    DepthStencil
  };

  struct AttachmentClearValue
  {
    using DepthValue = float;
    using StencilValue = uint32;
    using DepthStencilValue = Pair<DepthValue, StencilValue>;

    AttachmentClearValueType Type;
    Variant<ClearColourValue, DepthValue, StencilValue, DepthStencilValue> Value;

    KRYS_NODISCARD constexpr static AttachmentClearValue Colour(const Maths::Vec4f &colour) noexcept
    {
      return AttachmentClearValue {.Type = AttachmentClearValueType::Colour,
                                   .Value = ClearColourValue {colour}};
    }

    KRYS_NODISCARD constexpr static AttachmentClearValue Colour(const Maths::Vec4i &colour) noexcept
    {
      return AttachmentClearValue {.Type = AttachmentClearValueType::Colour,
                                   .Value = ClearColourValue {colour}};
    }

    KRYS_NODISCARD constexpr static AttachmentClearValue Colour(const Maths::Vec4u &colour) noexcept
    {
      return AttachmentClearValue {.Type = AttachmentClearValueType::Colour,
                                   .Value = ClearColourValue {colour}};
    }

    KRYS_NODISCARD constexpr static AttachmentClearValue Depth(float depth) noexcept
    {
      return AttachmentClearValue {.Type = AttachmentClearValueType::Depth, .Value = depth};
    }

    KRYS_NODISCARD constexpr static AttachmentClearValue Stencil(uint32 stencil) noexcept
    {
      return AttachmentClearValue {.Type = AttachmentClearValueType::Stencil, .Value = stencil};
    }

    KRYS_NODISCARD constexpr static AttachmentClearValue DepthStencil(float depth, uint32 stencil) noexcept
    {
      return AttachmentClearValue {.Type = AttachmentClearValueType::DepthStencil,
                                   .Value = DepthStencilValue {depth, stencil}};
    }
  };

  struct AttachmentDesc
  {
    AttachmentType Type;
    PixelFormat Format;
    AttachmentLoadOp OnLoad = AttachmentLoadOp::DontCare;
    AttachmentStoreOp OnStore = AttachmentStoreOp::DontCare;
    AttachmentClearValue ClearValue;
  };

  struct RenderTargetDesc
  {
    uint32 Width = 0u;
    uint32 Height = 0u;
    uint32 Samples = 1u;
    List<AttachmentDesc> Attachments;
  };
}