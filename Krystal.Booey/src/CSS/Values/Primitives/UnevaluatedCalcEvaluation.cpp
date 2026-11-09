/*
 * Copyright (C) 2024 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "Krystal.Booey/CSS/Calc/SymbolTable.hpp"
#include "Krystal.Booey/CSS/Calc/Value.hpp"
#include "Krystal.Booey/CSS/Values/NoConversionDataRequiredTag.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/UnevaluatedCalc.hpp"
#include "Krystal.Booey/CSS/Values/ToLengthConversionData.hpp"
#include "Krystal.Booey/Style/Builder/BuilderState.hpp"

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
