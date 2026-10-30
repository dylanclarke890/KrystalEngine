#pragma once

#include "Krystal.Booey/CSS/Values/Shapes/RectFunction.hpp"
#include "Krystal.Booey/Style/Values/Shapes/InsetFunction.hpp"

namespace krys::boo::style
{
  // NOTE: There is no style::RectShape type. Style conversion of css::RectShape yields a style::Inset.
  template <>
  struct ToStyle<css::RectShape>
  {
    KRYS_NODISCARD Inset operator()(const css::RectShape &, const BuilderState &) noexcept;
  };
  template <>
  struct ToStyle<css::RectFunction>
  {
    KRYS_NODISCARD InsetFunction operator()(const css::RectFunction &, const BuilderState &) noexcept;
  };
}
