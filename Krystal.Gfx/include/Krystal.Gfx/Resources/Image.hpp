#pragma once

#include "Krystal.Lib/Types.hpp"

namespace Krys::Gfx
{
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

  enum class PixelFormat : uint8
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

  struct ImageDesc
  {
    ImageType Type;
    PixelFormat Format;
    uint32 Width;
    uint32 Height;
    uint32 Depth;
    uint32 MipLevels;
    uint32 ArrayLayers;
  };
}