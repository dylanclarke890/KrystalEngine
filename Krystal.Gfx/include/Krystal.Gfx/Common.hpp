#pragma once

#include "Krystal.Lib/Types.hpp"

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