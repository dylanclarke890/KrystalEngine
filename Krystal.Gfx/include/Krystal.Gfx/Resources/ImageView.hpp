#pragma once

#include "Krystal.Gfx/Handle.hpp"
#include "Krystal.Gfx/Resources/Image.hpp"
#include "Krystal.Lib/Types.hpp"

namespace Krys::Gfx
{
  struct SubResourceRange
  {
    uint BaseMipLevel = 0;
    uint MipLevelCount = 1;
    uint BaseArrayLayer = 0;
    uint LayerCount = 1;
  };

  struct ImageViewDesc
  {
    ImageHandle Image {};
    ImageType Target {ImageType::Image2D};
    ImageFormat Format {ImageFormat::R8};
    SubResourceRange SubResourceRange {};
  };
}