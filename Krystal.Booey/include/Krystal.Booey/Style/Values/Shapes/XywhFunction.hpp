#pragma once

#include "Krystal.Booey/CSS/Values/Shapes/XywhFunction.hpp"
#include "Krystal.Booey/Style/Values/Shapes/InsetFunction.hpp"

namespace krys::boo::style
{
  // NOTE: There is no style::Xywh type. Style conversion of css::Xywh yields a style::Inset.
  template <>
  struct ToStyle<css::Xywh>
  {
    KRYS_NODISCARD Inset operator()(const css::Xywh &, const BuilderState &) noexcept;
  };
  template <>
  struct ToStyle<css::XywhFunction>
  {
    KRYS_NODISCARD InsetFunction operator()(const css::XywhFunction &, const BuilderState &) noexcept;
  };
}
