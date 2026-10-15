#pragma once

#include "Krystal.Booey/Style/Values/Primitives/NumericTypes.hpp"

namespace krys::boo::style
{
  // <'glyph-orientation-horizontal'> = <angle>
  // NOTE: The value of the angle is restricted to 0, 90, 180, and 270 degrees.
  // https://www.w3.org/TR/SVG11/text.html#GlyphOrientationHorizontalProperty
  enum class SVGGlyphOrientationHorizontal : uint8
  {
    Degrees0,
    Degrees90,
    Degrees180,
    Degrees270
  };

  // MARK: - Value Representation

  template <>
  struct ValueRepresentation<SVGGlyphOrientationHorizontal>
  {
    template <typename... F>
    decltype(auto) operator()(SVGGlyphOrientationHorizontal value, F &&...f)
    {
      auto visitor = krys::CreateVisitor(std::forward<F>(f)...);
      switch (value)
      {
        case SVGGlyphOrientationHorizontal::Degrees0:   return visitor(Angle<> {0});
        case SVGGlyphOrientationHorizontal::Degrees90:  return visitor(Angle<> {90});
        case SVGGlyphOrientationHorizontal::Degrees180: return visitor(Angle<> {180});
        case SVGGlyphOrientationHorizontal::Degrees270: return visitor(Angle<> {270});
      }
      krys_unreachable();
    }
  };

  // MARK: - Conversion

  template <>
  struct CSSValueConversion<SVGGlyphOrientationHorizontal>
  {
    KRYS_NODISCARD SVGGlyphOrientationHorizontal operator()(BuilderState &, const css::Value &) noexcept;
  };
}
