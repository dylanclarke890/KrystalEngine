#include "Krystal.Booey/Style/Values/FilterEffects/BlurFunction.hpp"

#include "Krystal.Booey/CSS/Values/FilterEffects/BlurFunction.hpp"
#include "Krystal.Booey/CSS/Values/FilterEffects/FilterFunctionDescriptor.hpp"
#include "Krystal.Booey/Graphics/Filters/FilterOperation.hpp"
#include "Krystal.Booey/Style/BuilderState.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesConversions.hpp"

namespace krys::boo::style
{
  KRYS_NODISCARD css::Blur ToCSSBlur(const BlurFilterOperation &operation, const RenderStyle &style) noexcept
  {
    return {css::Blur::Parameter {ConvertToCSS(Length<css::NonNegative> {operation.StdDeviation()}, style)}};
  }

  KRYS_NODISCARD Ref<FilterOperation> CreateFilterOperation(const css::Blur &filter,
                                                            const BuilderState &state) noexcept
  {
    float stdDeviation = 0;
    if (auto parameter = filter.Value)
    {
      stdDeviation = ConvertToStyle(*parameter, state).ResolveZoom(style::ZoomNeeded {});
    }
    else
    {
      stdDeviation = static_cast<float>(css::FilterFunctionDefaultValue<css::BlurFunction::Name>().Value);
    }

    return BlurFilterOperation::Create(stdDeviation);
  }
}
