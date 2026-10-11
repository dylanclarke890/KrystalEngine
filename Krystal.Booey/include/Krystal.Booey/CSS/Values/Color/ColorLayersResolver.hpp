#pragma once

#include "Krystal.Booey/Graphics/Color/Color.hpp"
#include "Krystal.Core/Types/SmallList.hpp"

namespace krys::boo
{
  enum class BlendMode : uint8;
}

namespace krys::boo::css
{
  struct ColorLayersResolver
  {
    BlendMode BlendMode;
    SmallList<boo::Color> Colors;
  };

  KRYS_NODISCARD boo::Color BlendSourceOver(const ColorLayersResolver &) noexcept;
}