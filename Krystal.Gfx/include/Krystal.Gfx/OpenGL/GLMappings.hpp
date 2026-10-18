#pragma once

#include "Krystal.Core/Assert.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Gfx/CompareOp.hpp"
#include "Krystal.Gfx/Formats.hpp"
#include "Krystal.Gfx/OpenGL/gl.hpp"
#include "Krystal.Gfx/Pipeline.hpp"
#include "Krystal.Gfx/Sampler.hpp"
#include "Krystal.Gfx/Texture.hpp"

namespace krys::gfx::opengl
{
  /// @brief The pixel transfer format and type for uploading and reading a texture format.
  struct GLTransferFormat
  {
    GLenum Format;
    GLenum Type;
  };

  KRYS_NODISCARD constexpr GLenum MapInternalFormat(TextureFormat format) noexcept
  {
    switch (format)
    {
      case TextureFormat::Undefined:            return GL_NONE;
      case TextureFormat::R8Unorm:              return GL_R8;
      case TextureFormat::RG8Unorm:             return GL_RG8;
      case TextureFormat::RGBA8Unorm:           return GL_RGBA8;
      case TextureFormat::RGBA8Srgb:            return GL_SRGB8_ALPHA8;
      case TextureFormat::BGRA8Unorm:           return GL_RGBA8;
      case TextureFormat::BGRA8Srgb:            return GL_SRGB8_ALPHA8;
      case TextureFormat::R16Float:             return GL_R16F;
      case TextureFormat::RG16Float:            return GL_RG16F;
      case TextureFormat::RGBA16Float:          return GL_RGBA16F;
      case TextureFormat::R32Float:             return GL_R32F;
      case TextureFormat::RG32Float:            return GL_RG32F;
      case TextureFormat::RGBA32Float:          return GL_RGBA32F;
      case TextureFormat::R32Uint:              return GL_R32UI;
      case TextureFormat::Depth16Unorm:         return GL_DEPTH_COMPONENT16;
      case TextureFormat::Depth24UnormStencil8: return GL_DEPTH24_STENCIL8;
      case TextureFormat::Depth32Float:         return GL_DEPTH_COMPONENT32F;
    }
    krys_unreachable();
  }

  KRYS_NODISCARD constexpr GLTransferFormat MapTransferFormat(TextureFormat format) noexcept
  {
    switch (format)
    {
      case TextureFormat::Undefined:            return {GL_NONE, GL_NONE};
      case TextureFormat::R8Unorm:              return {GL_RED, GL_UNSIGNED_BYTE};
      case TextureFormat::RG8Unorm:             return {GL_RG, GL_UNSIGNED_BYTE};
      case TextureFormat::RGBA8Unorm:
      case TextureFormat::RGBA8Srgb:            return {GL_RGBA, GL_UNSIGNED_BYTE};
      case TextureFormat::BGRA8Unorm:
      case TextureFormat::BGRA8Srgb:            return {GL_BGRA, GL_UNSIGNED_BYTE};
      case TextureFormat::R16Float:             return {GL_RED, GL_HALF_FLOAT};
      case TextureFormat::RG16Float:            return {GL_RG, GL_HALF_FLOAT};
      case TextureFormat::RGBA16Float:          return {GL_RGBA, GL_HALF_FLOAT};
      case TextureFormat::R32Float:             return {GL_RED, GL_FLOAT};
      case TextureFormat::RG32Float:            return {GL_RG, GL_FLOAT};
      case TextureFormat::RGBA32Float:          return {GL_RGBA, GL_FLOAT};
      case TextureFormat::R32Uint:              return {GL_RED_INTEGER, GL_UNSIGNED_INT};
      case TextureFormat::Depth16Unorm:         return {GL_DEPTH_COMPONENT, GL_UNSIGNED_SHORT};
      case TextureFormat::Depth24UnormStencil8: return {GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8};
      case TextureFormat::Depth32Float:         return {GL_DEPTH_COMPONENT, GL_FLOAT};
    }
    krys_unreachable();
  }

  KRYS_NODISCARD constexpr GLenum MapTextureTarget(TextureDimension dimension, uint32 samples) noexcept
  {
    switch (dimension)
    {
      case TextureDimension::Texture1D:      return GL_TEXTURE_1D;
      case TextureDimension::Texture2D:      return samples > 1u ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D;
      case TextureDimension::Texture2DArray: return GL_TEXTURE_2D_ARRAY;
      case TextureDimension::Texture3D:      return GL_TEXTURE_3D;
      case TextureDimension::Cube:           return GL_TEXTURE_CUBE_MAP;
    }
    krys_unreachable();
  }

  KRYS_NODISCARD constexpr GLenum MapCompareOp(CompareOp op) noexcept
  {
    switch (op)
    {
      case CompareOp::Never:          return GL_NEVER;
      case CompareOp::Less:           return GL_LESS;
      case CompareOp::Equal:          return GL_EQUAL;
      case CompareOp::LessOrEqual:    return GL_LEQUAL;
      case CompareOp::Greater:        return GL_GREATER;
      case CompareOp::NotEqual:       return GL_NOTEQUAL;
      case CompareOp::GreaterOrEqual: return GL_GEQUAL;
      case CompareOp::Always:         return GL_ALWAYS;
    }
    krys_unreachable();
  }

  KRYS_NODISCARD constexpr GLenum MapMinFilter(SamplerFilter filter, SamplerMipmapMode mipmapMode) noexcept
  {
    switch (mipmapMode)
    {
      case SamplerMipmapMode::None: return filter == SamplerFilter::Nearest ? GL_NEAREST : GL_LINEAR;
      case SamplerMipmapMode::Nearest:
        return filter == SamplerFilter::Nearest ? GL_NEAREST_MIPMAP_NEAREST : GL_LINEAR_MIPMAP_NEAREST;
      case SamplerMipmapMode::Linear:
        return filter == SamplerFilter::Nearest ? GL_NEAREST_MIPMAP_LINEAR : GL_LINEAR_MIPMAP_LINEAR;
    }
    krys_unreachable();
  }

  KRYS_NODISCARD constexpr GLenum MapMagFilter(SamplerFilter filter) noexcept
  {
    return filter == SamplerFilter::Nearest ? GL_NEAREST : GL_LINEAR;
  }

  KRYS_NODISCARD constexpr GLenum MapAddressMode(SamplerAddressMode mode) noexcept
  {
    switch (mode)
    {
      case SamplerAddressMode::Repeat:         return GL_REPEAT;
      case SamplerAddressMode::MirroredRepeat: return GL_MIRRORED_REPEAT;
      case SamplerAddressMode::ClampToEdge:    return GL_CLAMP_TO_EDGE;
      case SamplerAddressMode::ClampToBorder:  return GL_CLAMP_TO_BORDER;
    }
    krys_unreachable();
  }

  KRYS_NODISCARD constexpr GLenum MapTopology(PrimitiveTopology topology) noexcept
  {
    switch (topology)
    {
      case PrimitiveTopology::Points:        return GL_POINTS;
      case PrimitiveTopology::Lines:         return GL_LINES;
      case PrimitiveTopology::LineStrip:     return GL_LINE_STRIP;
      case PrimitiveTopology::Triangles:     return GL_TRIANGLES;
      case PrimitiveTopology::TriangleStrip: return GL_TRIANGLE_STRIP;
    }
    krys_unreachable();
  }

  KRYS_NODISCARD constexpr GLenum MapBlendFactor(BlendFactor factor) noexcept
  {
    switch (factor)
    {
      case BlendFactor::Zero:                      return GL_ZERO;
      case BlendFactor::One:                       return GL_ONE;
      case BlendFactor::SourceColour:              return GL_SRC_COLOR;
      case BlendFactor::OneMinusSourceColour:      return GL_ONE_MINUS_SRC_COLOR;
      case BlendFactor::DestinationColour:         return GL_DST_COLOR;
      case BlendFactor::OneMinusDestinationColour: return GL_ONE_MINUS_DST_COLOR;
      case BlendFactor::SourceAlpha:               return GL_SRC_ALPHA;
      case BlendFactor::OneMinusSourceAlpha:       return GL_ONE_MINUS_SRC_ALPHA;
      case BlendFactor::DestinationAlpha:          return GL_DST_ALPHA;
      case BlendFactor::OneMinusDestinationAlpha:  return GL_ONE_MINUS_DST_ALPHA;
      case BlendFactor::ConstantColour:            return GL_CONSTANT_COLOR;
      case BlendFactor::OneMinusConstantColour:    return GL_ONE_MINUS_CONSTANT_COLOR;
    }
    krys_unreachable();
  }

  KRYS_NODISCARD constexpr GLenum MapBlendOp(BlendOp op) noexcept
  {
    switch (op)
    {
      case BlendOp::Add:             return GL_FUNC_ADD;
      case BlendOp::Subtract:        return GL_FUNC_SUBTRACT;
      case BlendOp::ReverseSubtract: return GL_FUNC_REVERSE_SUBTRACT;
      case BlendOp::Min:             return GL_MIN;
      case BlendOp::Max:             return GL_MAX;
    }
    krys_unreachable();
  }

  KRYS_NODISCARD constexpr GLenum MapStencilOp(StencilOp op) noexcept
  {
    switch (op)
    {
      case StencilOp::Keep:              return GL_KEEP;
      case StencilOp::Zero:              return GL_ZERO;
      case StencilOp::Replace:           return GL_REPLACE;
      case StencilOp::IncrementAndClamp: return GL_INCR;
      case StencilOp::DecrementAndClamp: return GL_DECR;
      case StencilOp::Invert:            return GL_INVERT;
      case StencilOp::IncrementAndWrap:  return GL_INCR_WRAP;
      case StencilOp::DecrementAndWrap:  return GL_DECR_WRAP;
    }
    krys_unreachable();
  }

  /// @brief The component count and type of a vertex format, and whether it's read as integers.
  struct GLVertexFormat
  {
    GLint Components;
    GLenum Type;
    GLboolean Normalised;
    bool Integer;
  };

  KRYS_NODISCARD constexpr GLVertexFormat MapVertexFormat(VertexFormat format) noexcept
  {
    switch (format)
    {
      case VertexFormat::Float:      return {1, GL_FLOAT, GL_FALSE, false};
      case VertexFormat::Float2:     return {2, GL_FLOAT, GL_FALSE, false};
      case VertexFormat::Float3:     return {3, GL_FLOAT, GL_FALSE, false};
      case VertexFormat::Float4:     return {4, GL_FLOAT, GL_FALSE, false};
      case VertexFormat::Int:        return {1, GL_INT, GL_FALSE, true};
      case VertexFormat::Int2:       return {2, GL_INT, GL_FALSE, true};
      case VertexFormat::Int3:       return {3, GL_INT, GL_FALSE, true};
      case VertexFormat::Int4:       return {4, GL_INT, GL_FALSE, true};
      case VertexFormat::UInt:       return {1, GL_UNSIGNED_INT, GL_FALSE, true};
      case VertexFormat::UInt2:      return {2, GL_UNSIGNED_INT, GL_FALSE, true};
      case VertexFormat::UInt3:      return {3, GL_UNSIGNED_INT, GL_FALSE, true};
      case VertexFormat::UInt4:      return {4, GL_UNSIGNED_INT, GL_FALSE, true};
      case VertexFormat::UByte4:     return {4, GL_UNSIGNED_BYTE, GL_FALSE, true};
      case VertexFormat::UByte4Norm: return {4, GL_UNSIGNED_BYTE, GL_TRUE, false};
    }
    krys_unreachable();
  }
}
