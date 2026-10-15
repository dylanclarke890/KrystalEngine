#include "Krystal.Booey/Style/Values/Shapes/RectFunction.hpp"

#include "Krystal.Booey/Style/BuilderState.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesConversions.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesEvaluation.hpp"

namespace krys::boo::style
{
  using namespace css::Literals;

  // MARK: - Conversion

  Inset ToStyle<css::RectShape>::operator()(const css::RectShape &value, const BuilderState &state) noexcept
  {
    // "An auto value makes the edge of the box coincide with the corresponding edge of the
    //  reference box: it’s equivalent to 0% as the first (top) or fourth (left) value, and
    //  equivalent to 100% as the second (right) or third (bottom) value."
    //      (https://drafts.csswg.org/css-shapes-1/#funcdef-basic-shape-rect)

    // Conversion applies reflection to the trailing (right/bottom) edges to convert from rect()
    // form to inset() form. This means that all the `auto` values are converted to 0%.

    auto convertLeadingEdge =
      [&](const Variant<css::LengthPercentage<>, css::keywords::Auto> &edge) -> LengthPercentage<>
    {
      return krys::SwitchOn(
        edge, [&](const css::LengthPercentage<> &value) -> LengthPercentage<>
        { return ConvertToStyle(value, state); },
        [&](const css::keywords::Auto &) -> LengthPercentage<> { return 0_css_percentage; });
    };

    auto convertTrailingEdge =
      [&](const Variant<css::LengthPercentage<>, css::keywords::Auto> &edge) -> LengthPercentage<>
    {
      return krys::SwitchOn(
        edge, [&](const css::LengthPercentage<> &value) -> LengthPercentage<>
        { return Reflect(ConvertToStyle(value, state)); },
        [&](const css::keywords::Auto &) -> LengthPercentage<> { return 0_css_percentage; });
    };

    return {.Insets =
              {
                convertLeadingEdge(value.Edges.Top()),
                convertTrailingEdge(value.Edges.Right()),
                convertTrailingEdge(value.Edges.Bottom()),
                convertLeadingEdge(value.Edges.Left()),
              },
            .Radii = ConvertToStyle(value.Radii, state)};
  }

  InsetFunction ToStyle<css::RectFunction>::operator()(const css::RectFunction &value,
                                                       const BuilderState &state) noexcept
  {
    return {ConvertToStyle(value.Parameters, state)};
  }
}
