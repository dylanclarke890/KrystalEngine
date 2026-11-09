/*
 * Copyright (C) 2011, 2012 Google Inc. All rights reserved.
 * Copyright (C) 2014-2021 Apple Inc. All rights reserved.
 * Copyright (C) 2024 Samuel Weinig <sam@webkit.org>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following disclaimer
 * in the documentation and/or other materials provided with the
 * distribution.
 *     * Neither the name of Google Inc. nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "Krystal.Booey/CSS/Calc/SymbolTable.hpp"
#include "Krystal.Booey/CSS/Calc/TreeEvaluation.hpp"
#include "Krystal.Booey/CSS/Calc/TreeSimplification.hpp"
#include "Krystal.Booey/CSS/Calc/Value.hpp"
#include "Krystal.Booey/CSS/Values/NoConversionDataRequiredTag.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/NumericCategory.hpp"
#include "Krystal.Booey/CSS/Values/ToLengthConversionData.hpp"
#include "Krystal.Booey/Style/Calc/TreeConversion.hpp"
#include "Krystal.Booey/Style/Calc/Value.hpp"
#include "Krystal.Booey/Style/Values/Primitives/LengthResolution.hpp"

namespace krys::boo::css::calc
{
  Ref<Value> Value::Create(const style::calc::Value &value, const RenderStyle &style) noexcept
  {
    auto category = value.GetCategory();
    auto range = value.GetRange();

    auto toCSSOptions = style::calc::ToCSSOptions {
      .Category = category,
      .Range = range,
      .Style = style,
    };
    return Value::Create(category, range, style::calc::ToCSS(value.GetTree(), toCSSOptions));
  }

  Ref<Value> Value::CopySimplified(const ToLengthConversionData &conversionData) const noexcept
  {
    return CopySimplified(conversionData, {});
  }

  Ref<Value> Value::CopySimplified(const ToLengthConversionData &conversionData,
                                   const SymbolTable &symbolTable) const noexcept
  {
    auto simplificationOptions = SimplificationOptions {
      .Category = _category,
      .Range = _range,
      .ConversionData = conversionData,
      .SymbolTable = symbolTable,
      .AllowZeroValueLengthRemovalFromSum = true,
    };

    if (!CanSimplify(_tree, simplificationOptions))
    {
      return ShareRef(const_cast<Value &>(*this));
    }

    return Create(_category, _range, CopyAndSimplify(_tree, simplificationOptions));
  }

  Ref<Value> Value::CopySimplified(NoConversionDataRequiredTag tag) const noexcept
  {
    return CopySimplified(tag, {});
  }

  Ref<Value> Value::CopySimplified(NoConversionDataRequiredTag, const SymbolTable &symbolTable) const noexcept
  {
    auto simplificationOptions = SimplificationOptions {
      .Category = _category,
      .Range = _range,
      .ConversionData = null,
      .SymbolTable = symbolTable,
      .AllowZeroValueLengthRemovalFromSum = true,
    };

    if (!CanSimplify(_tree, simplificationOptions))
    {
      return ShareRef(const_cast<Value &>(*this));
    }

    return Create(_category, _range, CopyAndSimplify(_tree, simplificationOptions));
  }

  double Value::DoubleValue(const ToLengthConversionData &conversionData) const noexcept
  {
    return DoubleValue(conversionData, {});
  }

  double Value::DoubleValue(const ToLengthConversionData &conversionData,
                            const SymbolTable &symbolTable) const noexcept
  {
    auto options = EvaluationOptions {
      .Category = _category,
      .Range = _range,
      .ConversionData = conversionData,
      .SymbolTable = symbolTable,
    };
    return ClampToPermittedRange(EvaluateDouble(_tree, options).value_or(0));
  }

  double Value::DoubleValue(NoConversionDataRequiredTag tag) const noexcept
  {
    return DoubleValue(tag, {});
  }

  double Value::DoubleValue(NoConversionDataRequiredTag, const SymbolTable &symbolTable) const noexcept
  {
    auto options = EvaluationOptions {
      .Category = _category,
      .Range = _range,
      .ConversionData = null,
      .SymbolTable = symbolTable,
    };
    return ClampToPermittedRange(EvaluateDouble(_tree, options).value_or(0));
  }

  double Value::DoubleValueDeprecated() const noexcept
  {
    // FIXME: upstream logs an error here when the tree requires conversion data, as the result is then
    // likely incorrect.
    return DoubleValue(NoConversionDataRequiredTag {});
  }

  double Value::ComputeLengthPx(const ToLengthConversionData &conversionData) const noexcept
  {
    return ComputeLengthPx(conversionData, {});
  }

  double Value::ComputeLengthPx(const ToLengthConversionData &conversionData,
                                const SymbolTable &symbolTable) const noexcept
  {
    auto options = EvaluationOptions {
      .Category = _category,
      .Range = _range,
      .ConversionData = conversionData,
      .SymbolTable = symbolTable,
    };
    return ClampToPermittedRange(style::ComputeNonCalcLengthDouble(EvaluateDouble(_tree, options).value_or(0),
                                                                   LengthUnit::Px, conversionData));
  }

  Ref<style::calc::Value>
    Value::CreateCalculationValue(const ToLengthConversionData &conversionData) const noexcept
  {
    return CreateCalculationValue(conversionData, {});
  }

  Ref<style::calc::Value> Value::CreateCalculationValue(const ToLengthConversionData &conversionData,
                                                        const SymbolTable &symbolTable) const noexcept
  {
    auto toStyleOptions = style::calc::ToStyleOptions {
      .Category = _category,
      .Range = _range,
      .ConversionData = conversionData,
      .SymbolTable = symbolTable,
    };
    return style::calc::Value::Create(_category, _range, style::calc::ToStyle(_tree, toStyleOptions));
  }

  Ref<style::calc::Value> Value::CreateCalculationValue(NoConversionDataRequiredTag tag) const noexcept
  {
    return CreateCalculationValue(tag, {});
  }

  Ref<style::calc::Value> Value::CreateCalculationValue(NoConversionDataRequiredTag,
                                                        const SymbolTable &symbolTable) const noexcept
  {
    krys_debug_assert(!_tree.RequiresConversionData);

    auto toStyleOptions = style::calc::ToStyleOptions {
      .Category = _category,
      .Range = _range,
      .ConversionData = null,
      .SymbolTable = symbolTable,
    };
    return style::calc::Value::Create(_category, _range, style::calc::ToStyle(_tree, toStyleOptions));
  }
}
