#include "Krystal.Booey/Style/Values/FilterEffects/ContrastFunction.hpp"

#include "Krystal.Booey/CSS/Values/FilterEffects/ContrastFunction.hpp"
#include "Krystal.Booey/CSS/Values/FilterEffects/FilterFunctionDescriptor.hpp"
#include "Krystal.Booey/Graphics/Filters/FilterOperation.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesConversions.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesEvaluation.hpp"

namespace krys::boo::style
{
  KRYS_NODISCARD css::Contrast ToCSSContrast(const BasicComponentTransferFilterOperation &operation,
                                             const RenderStyle &style) noexcept
  {
    return {css::Contrast::Parameter {ConvertToCSS(Number<css::NonNegative> {operation.Amount()}, style)}};
  }

  KRYS_NODISCARD Ref<FilterOperation> CreateFilterOperation(const css::Contrast &filter,
                                                            const BuilderState &state) noexcept
  {
    double value;
    if (auto parameter = filter.Value)
    {
      value = Evaluate<double>(ConvertToStyle(*parameter, state));
    }
    else
    {
      value = css::FilterFunctionDefaultValue<css::ContrastFunction::Name>().Value;
    }

    return BasicComponentTransferFilterOperation::Create(
      value, css::FilterFunctionOperationType<css::ContrastFunction::Name>());
  }
}
