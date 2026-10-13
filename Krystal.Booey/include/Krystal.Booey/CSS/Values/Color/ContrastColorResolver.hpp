#pragma once

#include "Krystal.Booey/Graphics/Color/Color.hpp"

namespace krys::boo::css
{
  struct ContrastColorResolver
  {
    boo::Color Color {};
  };

  KRYS_NODISCARD boo::Color Resolve(const ContrastColorResolver &resolver) noexcept;
}
