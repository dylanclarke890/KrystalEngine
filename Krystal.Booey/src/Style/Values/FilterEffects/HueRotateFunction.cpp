#include "Krystal.Booey/Style/Values/FilterEffects/HueRotateFunction.hpp"

#include "Krystal.Booey/CSS/Values/FilterEffects/FilterFunctionDescriptor.hpp"
#include "Krystal.Booey/CSS/Values/FilterEffects/HueRotateFunction.hpp"
#include "Krystal.Booey/Graphics/Filters/FilterOperation.hpp"
#include "Krystal.Booey/Style/BuilderState.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesConversions.hpp"

namespace krys::boo::style
{
  KRYS_NODISCARD css::HueRotate ToCSSHueRotate(const BasicColorMatrixFilterOperation &operation,
                                               const RenderStyle &style) noexcept
  {
    return {css::HueRotate::Parameter {ConvertToCSS(Angle<> {operation.Amount()}, style)}};
  }

  KRYS_NODISCARD Ref<FilterOperation> CreateFilterOperation(const css::HueRotate &filter,
                                                            const BuilderState &state) noexcept
  {
    double value;
    if (auto parameter = filter.Value)
    {
      value = ConvertToStyle(*parameter, state).Value;
    }
    else
    {
      value = css::FilterFunctionDefaultValue<css::HueRotateFunction::Name>().Value;
    }

    return BasicColorMatrixFilterOperation::Create(
      value, css::FilterFunctionOperationType<css::HueRotateFunction::Name>());
  }
}
