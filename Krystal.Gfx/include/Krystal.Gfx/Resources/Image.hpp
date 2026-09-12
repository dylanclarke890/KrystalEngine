#pragma once

#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Gfx/Enums/ImageType.hpp"
#include "Krystal.Gfx/Enums/PixelFormat.hpp"

namespace krys::Gfx
{
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