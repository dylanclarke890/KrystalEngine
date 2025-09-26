#pragma once

#include "Krystal.Gfx/Common.hpp"

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