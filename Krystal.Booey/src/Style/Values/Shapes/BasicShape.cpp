#include "Krystal.Booey/Style/Values/Shapes/BasicShape.hpp"

#include "Krystal.Booey/CSS/Values/BasicShapeValue.hpp"
#include "Krystal.Booey/Style/BuilderState.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesBlending.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesConversions.hpp"

namespace krys::boo::style
{
  // MARK: - Conversion

  css::BasicShape ToCSS<BasicShape>::operator()(const BasicShape &value, const RenderStyle &style,
                                                PathConversion conversion) noexcept
  {
    return krys::SwitchOn(
      value, [&](const auto &shape) { return css::BasicShape {ConvertToCSS(shape, style)}; },
      [&](const PathFunction &path) { return css::BasicShape {ConvertToCSS(path, style, conversion)}; });
  }

  BasicShape ToStyle<css::BasicShape>::operator()(const css::BasicShape &value,
                                                  const BuilderState &builderState,
                                                  Maybe<float> zoom) noexcept
  {
    return krys::SwitchOn(
      value, [&](const auto &shape) { return BasicShape {ConvertToStyle(shape, builderState)}; },
      [&](const css::PathFunction &path) { return BasicShape {ConvertToStyle(path, builderState, zoom)}; });
  }

  Ref<css::Value> CSSValueCreation<BasicShape>::operator()(css::ValuePool &, const RenderStyle &style,
                                                           const BasicShape &value,
                                                           PathConversion conversion) noexcept
  {
    return css::BasicShapeValue::Create(ConvertToCSS(value, style, conversion));
  }

  BasicShape CSSValueConversion<BasicShape>::operator()(BuilderState &builderState, const css::Value &value,
                                                        Maybe<float> zoom) noexcept
  {
    return ConvertToStyle(Downcast<css::BasicShapeValue>(value).GetShape(), builderState, zoom);
  }

  // MARK: - Serialization

  void Serialiser<BasicShape>::operator()(css::CSSOMString &builder, const css::SerialisationContext &context,
                                          const RenderStyle &style, const BasicShape &value,
                                          PathConversion conversion) noexcept
  {
    css::Serialise(builder, context, ConvertToCSS(value, style, conversion));
  }

  // MARK: - Blending

  bool Blending<BasicShape>::CanBlend(const BasicShape &a, const BasicShape &b) noexcept
  {
    return std::visit(
      krys::CreateVisitor(
        []<typename T>(const T &a, const T &b) { return style::CanBlend(a, b); },
        [](const ShapeFunction &a, const PathFunction &b) { return CanBlendShapeWithPath(*a, *b); },
        [](const PathFunction &a, const ShapeFunction &b) { return CanBlendShapeWithPath(*b, *a); },
        [](const auto &, const auto &) { return false; }),
      a, b);
  }

  BasicShape Blending<BasicShape>::Blend(const BasicShape &a, const BasicShape &b,
                                         const BlendingContext &context) noexcept
  {
    return std::visit(
      krys::CreateVisitor(
        [&]<typename T>(const T &a, const T &b) -> BasicShape
        { return {krys::boo::style::Blend(a, b, context)}; },
        [&](const ShapeFunction &a, const PathFunction &b) -> BasicShape
        { return {krys::boo::style::Blend(a, ShapeFunction {*MakeShapeFromPath(*b)}, context)}; },
        [&](const PathFunction &a, const ShapeFunction &b) -> BasicShape
        { return {krys::boo::style::Blend(ShapeFunction {*MakeShapeFromPath(*a)}, b, context)}; },
        [&](const auto &, const auto &) -> BasicShape { krys_unreachable(); }),
      a, b);
  }

  // MARK: - Path

  boo::Path PathComputation<BasicShape>::operator()(const BasicShape &shape, const FloatRect &rect) noexcept
  {
    return krys::SwitchOn(shape,
                          [&](const auto &shape) { return krys::boo::style::ComputePath(shape, rect); });
  }

  // MARK: - Winding

  WindRule WindRuleComputation<BasicShape>::operator()(const BasicShape &shape) noexcept
  {
    return krys::SwitchOn(
      shape, [&](const style::PathFunction &path) { return krys::boo::style::ComputeWindRule(path); },
      [&](const style::PolygonFunction &polygon) { return krys::boo::style::ComputeWindRule(polygon); },
      [&](const style::ShapeFunction &shape) { return krys::boo::style::ComputeWindRule(shape); },
      [&](const auto &) { return WindRule::NonZero; });
  }
}
