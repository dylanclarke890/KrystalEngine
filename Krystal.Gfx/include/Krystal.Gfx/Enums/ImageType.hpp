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
}