#include "Krystal.Booey/Style/Values/FilterEffects/DropShadowFunction.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"

#include "Krystal.Booey/CSS/Values/FilterEffects/DropShadowFunction.hpp"
#include "Krystal.Booey/CSS/Values/PrimitiveValue.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/Graphics/Filters/FilterOperation.hpp"
#include "Krystal.Booey/Rendering/Style/DropShadowFilterOperationWithStyleColor.hpp"
#include "Krystal.Booey/Style/BuilderState.hpp"
#include "Krystal.Booey/Style/ForVisitedLink.hpp"
#include "Krystal.Booey/Style/Values/Color/Color.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesConversions.hpp"

namespace krys::boo::style
{
  KRYS_NODISCARD css::DropShadow ToCSSDropShadow(const DropShadowFilterOperationWithStyleColor &operation,
                                                 const RenderStyle &style) noexcept
  {
    return {
      .Color = ConvertToCSS(operation.StyleColor(), style),
      .Location = {ConvertToCSS(Length<css::AllUnzoomed> {static_cast<float>(operation.Location().X())},
                                style),
                   ConvertToCSS(Length<css::AllUnzoomed> {static_cast<float>(operation.Location().Y())},
                                style)},
      .StdDeviation =
        ConvertToCSS(Length<css::NonNegativeUnzoomed> {static_cast<float>(operation.StdDeviation())}, style),
    };
  }

  KRYS_NODISCARD Ref<FilterOperation> CreateFilterOperation(const css::DropShadow &filter,
                                                            const BuilderState &state) noexcept
  {
    const auto &zoomFactor = state.Style().UsedZoomForLength();
    int x = css::RoundForImpreciseConversion<int>(
      ConvertToStyle(get<0>(filter.Location), state).ResolveZoom(zoomFactor));
    int y = css::RoundForImpreciseConversion<int>(
      ConvertToStyle(get<1>(filter.Location), state).ResolveZoom(zoomFactor));
    int stdDeviation = filter.StdDeviation
                         ? css::RoundForImpreciseConversion<int>(
                             ConvertToStyle(*filter.StdDeviation, state).ResolveZoom(zoomFactor))
                         : 0;
    auto color =
      filter.Color ? ToStyleColor(*filter.Color, state, ForVisitedLink::No) : style::Color {CurrentColor {}};

    return DropShadowFilterOperationWithStyleColor::Create(IntPoint {x, y}, stdDeviation, color);
  }
}
