#include "Krystal.Booey/Style/Values/FilterEffects/GrayscaleFunction.hpp"

#include "Krystal.Booey/CSS/Values/FilterEffects/FilterFunctionDescriptor.hpp"
#include "Krystal.Booey/CSS/Values/FilterEffects/GrayscaleFunction.hpp"
#include "Krystal.Booey/Graphics/Filters/FilterOperation.hpp"
#include "Krystal.Booey/Style/BuilderState.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesConversions.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesEvaluation.hpp"

namespace krys::boo::style
{
  KRYS_NODISCARD css::Grayscale ToCSSGrayscale(const BasicColorMatrixFilterOperation &operation,
                                               const RenderStyle &style) noexcept
  {
    return {css::Grayscale::Parameter {
      ConvertToCSS(Number<css::ClosedUnitRangeClampUpper> {operation.Amount()}, style)}};
  }

  KRYS_NODISCARD Ref<FilterOperation> CreateFilterOperation(const css::Grayscale &filter,
                                                            const BuilderState &state) noexcept
  {
    double value;
    if (auto parameter = filter.Value)
    {
      value = Evaluate<double>(ConvertToStyle(*parameter, state));
    }
    else
    {
      value = css::FilterFunctionDefaultValue<css::GrayscaleFunction::Name>().Value;
    }

    return BasicColorMatrixFilterOperation::Create(
      value, css::FilterFunctionOperationType<css::GrayscaleFunction::Name>());
  }
}
