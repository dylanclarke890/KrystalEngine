#pragma once

#include "Krystal.Gfx/FontType.hpp"

namespace Krys::Gfx
{
  struct TextShaderDesc
  {
    FontType FontType {};
    bool EnableOutline {false};
    bool EnableShadow {false};
    bool EnableGlow {false};
  };
}