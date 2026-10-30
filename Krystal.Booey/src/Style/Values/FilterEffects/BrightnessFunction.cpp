#include "Krystal.Booey/Style/Values/FilterEffects/BrightnessFunction.hpp"

#include "Krystal.Booey/CSS/Values/FilterEffects/BrightnessFunction.hpp"
#include "Krystal.Booey/CSS/Values/FilterEffects/FilterFunctionDescriptor.hpp"
#include "Krystal.Booey/Graphics/Filters/FilterOperation.hpp"
#include "Krystal.Booey/Style/BuilderState.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesConversions.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesEvaluation.hpp"

namespace krys::boo::style
{
  KRYS_NODISCARD css::Brightness ToCSSBrightness(const BasicComponentTransferFilterOperation &operation,
                                                 const RenderStyle &style) noexcept
  {
    return {css::Brightness::Parameter {ConvertToCSS(Number<css::NonNegative> {operation.Amount()}, style)}};
  }

  KRYS_NODISCARD Ref<FilterOperation> CreateFilterOperation(const css::Brightness &filter,
                                                            const BuilderState &state) noexcept
  {
    double value;
    if (auto parameter = filter.Value)
    {
      value = Evaluate<double>(ConvertToStyle(*parameter, state));
    }
    else
    {
      value = css::FilterFunctionDefaultValue<css::BrightnessFunction::Name>().Value;
    }

    return BasicComponentTransferFilterOperation::Create(
      value, css::FilterFunctionOperationType<css::BrightnessFunction::Name>());
  }
}
