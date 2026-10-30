#include "Krystal.Booey/CSS/Calc/SymbolTable.hpp"
#include "Krystal.Booey/CSS/Calc/TreeEvaluation.hpp"
#include "Krystal.Booey/CSS/Calc/TreeSimplification.hpp"
#include "Krystal.Booey/CSS/Calc/Value.hpp"
#include "Krystal.Booey/CSS/ToLengthConversionData.hpp"
#include "Krystal.Booey/CSS/Values/NoConversionDataRequiredTag.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/NumericCategory.hpp"
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
