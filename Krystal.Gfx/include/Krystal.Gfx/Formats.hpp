#pragma once

#include "Krystal.Core/Assert.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Numeric.hpp"

namespace krys::gfx
{
  /// @brief The format of a texel in a texture or render target.
  enum class TextureFormat : uint8
  {
    Undefined,

    R8Unorm,
    RG8Unorm,
    RGBA8Unorm,
    RGBA8Srgb,
    BGRA8Unorm,
    BGRA8Srgb,

    R16Float,
    RG16Float,
    RGBA16Float,

    R32Float,
    RG32Float,
    RGBA32Float,

    R32Uint,

    Depth16Unorm,
    Depth24UnormStencil8,
    Depth32Float,
  };

  /// @brief The type of the indices in an index buffer.
  enum class IndexType : uint8
  {
    UInt16,
    UInt32,
  };

  /// @brief The format of a vertex attribute in a vertex buffer.
  enum class VertexFormat : uint8
  {
    Float,
    Float2,
    Float3,
    Float4,
    Int,
    Int2,
    Int3,
    Int4,
    UInt,
    UInt2,
    UInt3,
    UInt4,
    UByte4,
    UByte4Norm,
  };

  /// @brief Returns the size of one texel in bytes.
  KRYS_NODISCARD constexpr uint32 BytesPerTexel(TextureFormat format) noexcept
  {
    switch (format)
    {
      case TextureFormat::Undefined:            return 0u;
      case TextureFormat::R8Unorm:              return 1u;
      case TextureFormat::RG8Unorm:             return 2u;
      case TextureFormat::RGBA8Unorm:
      case TextureFormat::RGBA8Srgb:
      case TextureFormat::BGRA8Unorm:
      case TextureFormat::BGRA8Srgb:            return 4u;
      case TextureFormat::R16Float:             return 2u;
      case TextureFormat::RG16Float:            return 4u;
      case TextureFormat::RGBA16Float:          return 8u;
      case TextureFormat::R32Float:             return 4u;
      case TextureFormat::RG32Float:            return 8u;
      case TextureFormat::RGBA32Float:          return 16u;
      case TextureFormat::R32Uint:              return 4u;
      case TextureFormat::Depth16Unorm:         return 2u;
      case TextureFormat::Depth24UnormStencil8: return 4u;
      case TextureFormat::Depth32Float:         return 4u;
    }
    krys_unreachable();
  }

  /// @brief Whether the format holds depth (and possibly stencil) values.
  KRYS_NODISCARD constexpr bool IsDepthFormat(TextureFormat format) noexcept
  {
    return format == TextureFormat::Depth16Unorm || format == TextureFormat::Depth24UnormStencil8
           || format == TextureFormat::Depth32Float;
  }

  /// @brief Whether the format holds stencil values.
  KRYS_NODISCARD constexpr bool HasStencil(TextureFormat format) noexcept
  {
    return format == TextureFormat::Depth24UnormStencil8;
  }

  /// @brief Whether the colour channels are sRGB encoded, and converted to and from linear on access.
  KRYS_NODISCARD constexpr bool IsSrgbFormat(TextureFormat format) noexcept
  {
    return format == TextureFormat::RGBA8Srgb || format == TextureFormat::BGRA8Srgb;
  }

  /// @brief Whether the format holds unsigned integers that shaders read without normalisation.
  KRYS_NODISCARD constexpr bool IsIntegerFormat(TextureFormat format) noexcept
  {
    return format == TextureFormat::R32Uint;
  }

  /// @brief Returns the size of one index in bytes.
  KRYS_NODISCARD constexpr uint32 IndexSize(IndexType type) noexcept
  {
    return type == IndexType::UInt16 ? 2u : 4u;
  }

  /// @brief Returns the size of a vertex attribute in bytes.
  KRYS_NODISCARD constexpr uint32 VertexFormatSize(VertexFormat format) noexcept
  {
    switch (format)
    {
      case VertexFormat::Float:
      case VertexFormat::Int:
      case VertexFormat::UInt:
      case VertexFormat::UByte4:
      case VertexFormat::UByte4Norm: return 4u;
      case VertexFormat::Float2:
      case VertexFormat::Int2:
      case VertexFormat::UInt2:      return 8u;
      case VertexFormat::Float3:
      case VertexFormat::Int3:
      case VertexFormat::UInt3:      return 12u;
      case VertexFormat::Float4:
      case VertexFormat::Int4:
      case VertexFormat::UInt4:      return 16u;
    }
    krys_unreachable();
  }
}
