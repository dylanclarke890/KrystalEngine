#pragma once

#include "Krystal.Booey/Style/Computed/ComputedStyle.hpp"

#define COMPUTED_STYLE_PROPERTIES_INITIAL_INLINES_INCLUDE_TRAP 1
#include "Krystal.Booey/Style/Computed/ComputedStyleProperties+InitialInlines.hpp"
#undef COMPUTED_STYLE_PROPERTIES_INITIAL_INLINES_INCLUDE_TRAP

namespace krys::boo::style
{
  // MARK: - Non-property initial values

  constexpr ZIndex ComputedStyle::InitialUsedZIndex() noexcept
  {
    return css::keywords::Auto {};
  }

  inline PageSize ComputedStyle::InitialPageSize() noexcept
  {
    return css::keywords::Auto {};
  }
}
