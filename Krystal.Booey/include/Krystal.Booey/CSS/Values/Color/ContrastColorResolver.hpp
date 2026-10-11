#pragma once

#include "Krystal.Booey/Graphics/Color/Color.hpp"

namespace krys::boo::css
{
  struct ContrastColorResolver
  {
    boo::Color color {};
  };

  boo::Color Resolve(const ContrastColorResolver &) noexcept;
}