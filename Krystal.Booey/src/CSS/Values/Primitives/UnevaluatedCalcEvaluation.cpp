#include "Krystal.Booey/CSS/Calc/SymbolTable.hpp"
#include "Krystal.Booey/CSS/Calc/Value.hpp"
#include "Krystal.Booey/CSS/ToLengthConversionData.hpp"
#include "Krystal.Booey/CSS/Values/NoConversionDataRequiredTag.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/UnevaluatedCalc.hpp"
#include "Krystal.Booey/Style/BuilderState.hpp"
#include "Krystal.Booey/Style/Seam.hpp"

namespace krys::boo::css
{
  UnevaluatedCalcBase UnevaluatedCalcBase::SimplifyBase(const ToLengthConversionData &conversionData,
                                                        const SymbolTable &symbolTable) const noexcept
  {
    return UnevaluatedCalcBase {ProtectedCalc()->CopySimplified(conversionData, symbolTable)};
  }

  double UnevaluatedCalcBase::Evaluate(Category category, const style::BuilderState &state) const noexcept
  {
    return Evaluate(category, state.CssToLengthConversionData(), {});
  }

  double UnevaluatedCalcBase::Evaluate(Category category, const style::BuilderState &state,
                                       const SymbolTable &symbolTable) const noexcept
  {
    return Evaluate(category, state.CssToLengthConversionData(), symbolTable);
  }

  double UnevaluatedCalcBase::Evaluate(Category category,
                                       const ToLengthConversionData &conversionData) const noexcept
  {
    return Evaluate(category, conversionData, {});
  }

  double UnevaluatedCalcBase::Evaluate(KRYS_MAYBE_UNUSED Category category,
                                       const ToLengthConversionData &conversionData,
                                       const SymbolTable &symbolTable) const noexcept
  {
    krys_debug_assert(ProtectedCalc()->GetCategory() == category);
    return ProtectedCalc()->DoubleValue(conversionData, symbolTable);
  }

  double UnevaluatedCalcBase::Evaluate(Category category, NoConversionDataRequiredTag tag) const noexcept
  {
    return Evaluate(category, tag, {});
  }

  double UnevaluatedCalcBase::Evaluate(KRYS_MAYBE_UNUSED Category category, NoConversionDataRequiredTag tag,
                                       const SymbolTable &symbolTable) const noexcept
  {
    krys_debug_assert(ProtectedCalc()->GetCategory() == category);
    return ProtectedCalc()->DoubleValue(tag, symbolTable);
  }
}
