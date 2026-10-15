#pragma once

#include "Krystal.Booey/Style/Computed/ComputedStyle.hpp"
#include "Krystal.Booey/Style/Computed/ComputedStyleBase+ConstructionInlines.hpp"

namespace krys::boo::style
{
  inline ComputedStyle::ComputedStyle(ComputedStyle &&) noexcept = default;
  inline ComputedStyle &ComputedStyle::operator=(ComputedStyle &&) = default;

  inline ComputedStyle::ComputedStyle(CreateDefaultStyleTag tag) noexcept : ComputedStyleProperties {tag}
  {
  }

  inline ComputedStyle::ComputedStyle(const ComputedStyle &other, CloneTag tag) noexcept
      : ComputedStyleProperties {other, tag}
  {
  }

  inline ComputedStyle::ComputedStyle(ComputedStyle &a, ComputedStyle &&b) noexcept
      : ComputedStyleProperties {a, krys::move(b)}
  {
  }
}
