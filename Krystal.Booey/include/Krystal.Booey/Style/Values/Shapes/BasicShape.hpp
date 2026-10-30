#pragma once

#include "Krystal.Booey/CSS/Values/Shapes/BasicShape.hpp"
#include "Krystal.Booey/Style/Values/Shapes/CircleFunction.hpp"
#include "Krystal.Booey/Style/Values/Shapes/EllipseFunction.hpp"
#include "Krystal.Booey/Style/Values/Shapes/InsetFunction.hpp"
#include "Krystal.Booey/Style/Values/Shapes/PathComputation.hpp"
#include "Krystal.Booey/Style/Values/Shapes/PathFunction.hpp"
#include "Krystal.Booey/Style/Values/Shapes/PolygonFunction.hpp"
#include "Krystal.Booey/Style/Values/Shapes/RectFunction.hpp"
#include "Krystal.Booey/Style/Values/Shapes/ShapeFunction.hpp"
#include "Krystal.Booey/Style/Values/Shapes/WindRuleComputation.hpp"
#include "Krystal.Booey/Style/Values/Shapes/XywhFunction.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"

namespace krys::boo::style
{
  // NOTE: This differs from css::BasicShape due to lack of RectFunction and XywhFunction, both of
  // which convert to InsetFunction during style conversion.
  using BasicShape =
    Variant<CircleFunction, EllipseFunction, InsetFunction, PathFunction, PolygonFunction, ShapeFunction>;

  template <typename T>
  concept ShapeWithCenterCoordinate = std::same_as<T, CircleFunction> || std::same_as<T, EllipseFunction>;

  // MARK: - Conversion

  template <>
  struct ToCSS<BasicShape>
  {
    KRYS_NODISCARD css::BasicShape operator()(const BasicShape &, const RenderStyle &,
                                              PathConversion = PathConversion::None) noexcept;
  };
  template <>
  struct ToStyle<css::BasicShape>
  {
    KRYS_NODISCARD BasicShape operator()(const css::BasicShape &, const BuilderState &,
                                         Maybe<float> zoom = 1.0f) noexcept;
  };

  template <>
  struct CSSValueCreation<BasicShape>
  {
    KRYS_NODISCARD Ref<css::Value> operator()(css::ValuePool &, const RenderStyle &, const BasicShape &,
                                              PathConversion = PathConversion::None) noexcept;
  };
  template <>
  struct CSSValueConversion<BasicShape>
  {
    KRYS_NODISCARD BasicShape operator()(BuilderState &, const css::Value &,
                                         Maybe<float> zoom = 1.0f) noexcept;
  };

  // MARK: - Serialization

  template <>
  struct Serialiser<BasicShape>
  {
    void operator()(StringBuilder &, const css::SerialisationContext &, const RenderStyle &,
                    const BasicShape &, PathConversion = PathConversion::None) noexcept;
  };

  // MARK: - Blending

  template <>
  struct Blending<BasicShape>
  {
    KRYS_NODISCARD bool CanBlend(const BasicShape &, const BasicShape &) noexcept;
    KRYS_NODISCARD BasicShape Blend(const BasicShape &, const BasicShape &, const BlendingContext &) noexcept;
  };

  // MARK: - Path

  template <>
  struct PathComputation<BasicShape>
  {
    KRYS_NODISCARD boo::Path operator()(const BasicShape &, const FloatRect &) noexcept;
  };

  // MARK: - Winding

  template <>
  struct WindRuleComputation<BasicShape>
  {
    KRYS_NODISCARD boo::WindRule operator()(const BasicShape &) noexcept;
  };
}
