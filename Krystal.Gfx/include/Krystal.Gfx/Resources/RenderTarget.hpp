#pragma once

#include "Krystal.Gfx/Resources/Image.hpp"
#include "Krystal.Lib/List.hpp"
#include "Krystal.Lib/Types.hpp"
#include "Krystal.Maths/Vector.hpp"
#include <variant>

namespace Krys::Gfx
{

  enum class AttachmentType
  {
    Colour,
    Depth,
    Stencil,
    DepthStencil
  };

  enum class AttachmentLoadOp : uint8
  {
    Load,
    Clear,
    DontCare
  };

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
  };

  struct AttachmentClearValue
  {
    using DepthValue = float;
    using StencilValue = uint32;

    AttachmentClearValueType Type;
    std::variant<ClearColourValue, DepthValue, StencilValue> Value;

    inline static AttachmentClearValue Colour(const Maths::Vec4f &colour) noexcept
    {
      return AttachmentClearValue {.Type = AttachmentClearValueType::Colour,
                                   .Value = ClearColourValue {colour}};
    }

    inline static AttachmentClearValue Colour(const Maths::Vec4i &colour) noexcept
    {
      return AttachmentClearValue {.Type = AttachmentClearValueType::Colour,
                                   .Value = ClearColourValue {colour}};
    }

    inline static AttachmentClearValue Colour(const Maths::Vec4u &colour) noexcept
    {
      return AttachmentClearValue {.Type = AttachmentClearValueType::Colour,
                                   .Value = ClearColourValue {colour}};
    }

    inline static AttachmentClearValue Depth(float depth) noexcept
    {
      return AttachmentClearValue {.Type = AttachmentClearValueType::Depth, .Value = depth};
    }

    inline static AttachmentClearValue Stencil(uint32 stencil) noexcept
    {
      return AttachmentClearValue {.Type = AttachmentClearValueType::Stencil, .Value = stencil};
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