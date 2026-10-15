#include "Krystal.Booey/Style/Values/FilterEffects/SaturateFunction.hpp"

#include "Krystal.Booey/CSS/Values/FilterEffects/FilterFunctionDescriptor.hpp"
#include "Krystal.Booey/CSS/Values/FilterEffects/SaturateFunction.hpp"
#include "Krystal.Booey/Graphics/Filters/FilterOperation.hpp"
#include "Krystal.Booey/Style/BuilderState.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesConversions.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesEvaluation.hpp"

namespace krys::boo::style
{
  KRYS_NODISCARD css::Saturate ToCSSSaturate(const BasicColorMatrixFilterOperation &operation,
                                             const RenderStyle &style) noexcept
  {
    return {css::Saturate::Parameter {ConvertToCSS(Number<css::NonNegative> {operation.Amount()}, style)}};
  }

  KRYS_NODISCARD Ref<FilterOperation> CreateFilterOperation(const css::Saturate &filter,
                                                            const BuilderState &state) noexcept
  {
    double value;
    if (auto parameter = filter.Value)
    {
      value = Evaluate<double>(ConvertToStyle(*parameter, state));
    }
    else
    {
      value = css::FilterFunctionDefaultValue<css::SaturateFunction::Name>().Value;
    }

    return BasicColorMatrixFilterOperation::Create(
      value, css::FilterFunctionOperationType<css::SaturateFunction::Name>());
  }
}
