#pragma once

#include "Krystal.Lib/Types.hpp"
#include "Krystal.Gfx/Enums/ImageType.hpp"
#include "Krystal.Gfx/Enums/PixelFormat.hpp"

namespace Krys::Gfx
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