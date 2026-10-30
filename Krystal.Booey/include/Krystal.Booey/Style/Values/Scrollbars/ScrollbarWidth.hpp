#pragma once

#include "Krystal.Booey/Core/ScrollTypes.hpp"
#include "Krystal.Booey/Style/Values/Types.hpp"

namespace krys::boo::style
{
  // <'scrollbar-width'> = auto | thin | none
  // https://drafts.csswg.org/css-scrollbars/#propdef-scrollbar-width
  enum class ScrollbarWidth : uint8
  {
    Auto,
    Thin,
    None
  };

  // MARK: - Conversion

  // `ScrollbarWidth` is special-cased to apply `needsScrollbarWidthThinDisabledQuirk` quirk.
  template <>
  struct CSSValueConversion<ScrollbarWidth>
  {
    KRYS_NODISCARD ScrollbarWidth operator()(BuilderState &, const css::Value &) noexcept;
  };

  // MARK: - Platform

  template <>
  struct ToPlatform<ScrollbarWidth>
  {
    KRYS_NODISCARD boo::ScrollbarWidth operator()(ScrollbarWidth value) noexcept
    {
      switch (value)
      {
        case ScrollbarWidth::Auto: return boo::ScrollbarWidth::Auto;
        case ScrollbarWidth::Thin: return boo::ScrollbarWidth::Thin;
        case ScrollbarWidth::None: return boo::ScrollbarWidth::None;
      }
      krys_unreachable();
    }
  };
}
