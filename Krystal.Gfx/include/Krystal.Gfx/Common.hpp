#pragma once

#include "Krystal.Lib/List.hpp"
#include "Krystal.Lib/Types.hpp"
#include "Krystal.Maths/Vector.hpp"
#include <variant>

namespace Krys::Gfx
{
  enum class PrimitiveType : uint8
  {
    Points = 0,
    Lines,
    LineStrip,
    Triangles,
    TriangleStrip,
    TriangleFan,
  };

  enum class FilterMode : uint8
  {
    Nearest = 0,
    Linear,
    NearestMipmapNearest,
    LinearMipmapNearest,
    NearestMipmapLinear,
    LinearMipmapLinear,
  };

  enum class WrapMode : uint8
  {
    Repeat = 0,
    ClampToEdge,
    ClampToBorder,
  };

  enum class ImageType : uint8
  {
    Image1D,
    Image2D,
    Image3D,
    ImageCube,
    Image1DArray,
    Image2DArray,
    ImageCubeArray
  };

  enum class ImageFormat : uint8
  {
    R8,
    R8G8,
    R8G8B8,
    R8G8B8A8,
    SRGB8,
    SRGB8A8,
    R16F,
    R16G16F,
    R16G16B16F,
    R16G16B16A16F,
    R32F,
    R32G32F,
    R32G32B32F,
    R32G32B32A32F,
    DEPTH24STENCIL8,
    DEPTH32F
  };

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
    ImageFormat Format;
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

  enum class FontType : uint8
  {
    Bitmap,
    SDF,
    MSDF,
    MTSDF
  };

  struct SubResourceRange
  {
    uint BaseMipLevel = 0;
    uint MipLevelCount = 1;
    uint BaseArrayLayer = 0;
    uint LayerCount = 1;
  };
}