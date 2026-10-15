#pragma once

#include "Krystal.Booey/Style/Values/Primitives/NumericTypes.hpp"

namespace krys::boo::style
{
  // <'glyph-orientation-vertical'> = auto | <angle>
  // NOTE: The value of the angle is restricted to 0, 90, 180, and 270 degrees.
  // https://www.w3.org/TR/SVG11/text.html#GlyphOrientationVerticalProperty
  enum class SVGGlyphOrientationVertical : uint8
  {
    Auto,
    Degrees0,
    Degrees90,
    Degrees180,
    Degrees270
  };

  // MARK: - Value Representation

  template <>
  struct ValueRepresentation<SVGGlyphOrientationVertical>
  {
    template <typename... F>
    decltype(auto) operator()(SVGGlyphOrientationVertical value, F &&...f)
    {
      auto visitor = krys::CreateVisitor(std::forward<F>(f)...);
      switch (value)
      {
        case SVGGlyphOrientationVertical::Auto:       return visitor(css::keywords::Auto {});
        case SVGGlyphOrientationVertical::Degrees0:   return visitor(Angle<> {0});
        case SVGGlyphOrientationVertical::Degrees90:  return visitor(Angle<> {90});
        case SVGGlyphOrientationVertical::Degrees180: return visitor(Angle<> {180});
        case SVGGlyphOrientationVertical::Degrees270: return visitor(Angle<> {270});
      }
      krys_unreachable();
    }
  };

  // MARK: - Conversion

  template <>
  struct CSSValueConversion<SVGGlyphOrientationVertical>
  {
    KRYS_NODISCARD SVGGlyphOrientationVertical operator()(BuilderState &, const css::Value &) noexcept;
  };
}
