#include "Krystal.Booey/Style/Values/Shapes/XywhFunction.hpp"

#include "Krystal.Booey/Style/Values/Primitives/NumericTypesConversions.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesEvaluation.hpp"

namespace krys::boo::style
{
  Inset ToStyle<css::Xywh>::operator()(const css::Xywh &value, const BuilderState &state) noexcept
  {
    auto location = ConvertToStyle(value.Location, state);
    auto size = ConvertToStyle(value.Size, state);

    return {.Insets =
              {
                location.y(),
                ReflectSum(location.x(), size.Width()),
                ReflectSum(location.y(), size.Height()),
                location.x(),
              },
            .Radii = ConvertToStyle(value.Radii, state)};
  }

  InsetFunction ToStyle<css::XywhFunction>::operator()(const css::XywhFunction &value,
                                                       const BuilderState &state) noexcept
  {
    return {ConvertToStyle(value.Parameters, state)};
  }
}
