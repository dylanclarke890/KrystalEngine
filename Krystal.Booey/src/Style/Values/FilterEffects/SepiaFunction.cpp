#include "Krystal.Booey/Style/Values/FilterEffects/SepiaFunction.hpp"

#include "Krystal.Booey/CSS/Values/FilterEffects/FilterFunctionDescriptor.hpp"
#include "Krystal.Booey/CSS/Values/FilterEffects/SepiaFunction.hpp"
#include "Krystal.Booey/Graphics/Filters/FilterOperation.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesConversions.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesEvaluation.hpp"

namespace krys::boo::style
{
  KRYS_NODISCARD css::Sepia ToCSSSepia(const BasicColorMatrixFilterOperation &operation,
                                       const RenderStyle &style) noexcept
  {
    return {css::Sepia::Parameter {
      ConvertToCSS(Number<css::ClosedUnitRangeClampUpper> {operation.Amount()}, style)}};
  }

  KRYS_NODISCARD Ref<FilterOperation> CreateFilterOperation(const css::Sepia &filter,
                                                            const BuilderState &state) noexcept
  {
    double value;
    if (auto parameter = filter.Value)
    {
      value = Evaluate<double>(ConvertToStyle(*parameter, state));
    }
    else
    {
      value = css::FilterFunctionDefaultValue<css::SepiaFunction::Name>().Value;
    }

    return BasicColorMatrixFilterOperation::Create(
      value, css::FilterFunctionOperationType<css::SepiaFunction::Name>());
  }
}
