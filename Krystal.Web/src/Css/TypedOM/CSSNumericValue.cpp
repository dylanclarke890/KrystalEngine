/*
 * Copyright (C) 2019 Apple Inc. All rights reserved.
 * Copyright (C) 2025 Samuel Weinig <sam@webkit.org>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above
 *    copyright notice, this list of conditions and the following
 *    disclaimer.
 * 2. Redistributions in binary form must reproduce the above
 *    copyright notice, this list of conditions and the following
 *    disclaimer in the documentation and/or other materials
 *    provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER “AS IS” AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,
 * OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
 * TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
 * THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include "Krystal.Web/Css/TypedOM/CSSNumericValue.hpp"
#include "Krystal.Web/Css/Calc/SymbolTable.hpp"
#include "Krystal.Web/Css/Calc/Tree.hpp"
#include "Krystal.Web/Css/Calc/TreeParser.hpp"
#include "Krystal.Web/Css/Calc/TreeSimplification.hpp"
#include "Krystal.Web/Css/Parser/InputStream.hpp"
#include "Krystal.Web/Css/Parser/ParserContext.hpp"
#include "Krystal.Web/Css/Parser/Tokenizer.hpp"
#include "Krystal.Web/Css/Parser/TokenRange.hpp"
#include "Krystal.Web/Css/Properties/PropertyParserState.hpp"
#include "Krystal.Web/Css/TypedOM/CSSNumericFactory.hpp"
#include "Krystal.Web/Css/TypedOM/CSSUnitValue.hpp"
#include "Krystal.Web/Css/TypedOM/Numeric/CSSMathClamp.hpp"
#include "Krystal.Web/Css/TypedOM/Numeric/CSSMathInvert.hpp"
#include "Krystal.Web/Css/TypedOM/Numeric/CSSMathMax.hpp"
#include "Krystal.Web/Css/TypedOM/Numeric/CSSMathMin.hpp"
#include "Krystal.Web/Css/TypedOM/Numeric/CSSMathNegate.hpp"
#include "Krystal.Web/Css/TypedOM/Numeric/CSSMathProduct.hpp"
#include "Krystal.Web/Css/TypedOM/Numeric/CSSMathSum.hpp"
#include "Krystal.Web/Css/TypedOM/Numeric/CSSNumericArray.hpp"
#include <algorithm>
#include <functional>

// Explicitly prefixed name used to avoid conflicts with existing macros that can be indirectly included.
#define KRYS_CSS_NUMERIC_RETURN_IF_EXCEPTION(resultVariable, expression)                                     \
  auto resultOrException_##resultVariable = expression;                                                      \
  if (resultOrException_##resultVariable.HasException())                                                     \
  {                                                                                                          \
    return resultOrException_##resultVariable.ReleaseException();                                            \
  }                                                                                                          \
  auto resultVariable = resultOrException_##resultVariable.ReleaseValue()

namespace krys::web
{
  template <typename T>
  KRYS_NODISCARD static ExceptionOr<Ref<CSSNumericValue>>
    ConvertToExceptionOrNumericValue(ExceptionOr<Ref<T>> &&input) noexcept
  {
    KRYS_CSS_NUMERIC_RETURN_IF_EXCEPTION(result, krys::move(input));
    return Ref<CSSNumericValue> {krys::move(result)};
  }

  template <typename T>
  KRYS_NODISCARD static ExceptionOr<Ref<CSSNumericValue>>
    ConvertToExceptionOrNumericValue(Ref<T> &&input) noexcept
  {
    return Ref<CSSNumericValue> {krys::move(input)};
  }

  KRYS_NODISCARD static ExceptionOr<List<Ref<CSSNumericValue>>>
    ReifyMathExpressions(const css::calc::Children &nodes) noexcept
  {
    List<Ref<CSSNumericValue>> values;
    values.reserve(nodes.size());
    for (const auto &node : nodes)
    {
      KRYS_CSS_NUMERIC_RETURN_IF_EXCEPTION(value, CSSNumericValue::ReifyMathExpression(node));
      values.push_back(krys::move(value));
    }
    return values;
  }

  template <css::calc::Numeric T>
  KRYS_NODISCARD static ExceptionOr<Ref<CSSNumericValue>> ReifyMathExpression(const T &root) noexcept
  {
    return ConvertToExceptionOrNumericValue(CSSUnitValue::Create(root.Value, css::calc::ToUnit(root)));
  }

  KRYS_NODISCARD static ExceptionOr<Ref<CSSNumericValue>>
    ReifyMathExpression(const css::calc::Symbol &) noexcept
  {
    // CSS Typed OM doesn't currently support unresolved symbols.
    return Exception {ExceptionCode::UnknownError};
  }

  KRYS_NODISCARD static ExceptionOr<Ref<CSSNumericValue>>
    ReifyMathExpression(const css::calc::SiblingCount &) noexcept
  {
    // CSS Typed OM doesn't currently support unresolved sibling-count() functions.
    return Exception {ExceptionCode::UnknownError};
  }

  KRYS_NODISCARD static ExceptionOr<Ref<CSSNumericValue>>
    ReifyMathExpression(const css::calc::SiblingIndex &) noexcept
  {
    // CSS Typed OM doesn't currently support unresolved sibling-index() functions.
    return Exception {ExceptionCode::UnknownError};
  }

  KRYS_NODISCARD static ExceptionOr<Ref<CSSNumericValue>>
    ReifyMathExpression(const css::calc::IndirectNode<css::calc::Sum> &root) noexcept
  {
    KRYS_CSS_NUMERIC_RETURN_IF_EXCEPTION(values, ReifyMathExpressions(root->Children));
    return ConvertToExceptionOrNumericValue(CSSMathSum::Create(krys::move(values)));
  }

  KRYS_NODISCARD static ExceptionOr<Ref<CSSNumericValue>>
    ReifyMathExpression(const css::calc::IndirectNode<css::calc::Product> &root) noexcept
  {
    KRYS_CSS_NUMERIC_RETURN_IF_EXCEPTION(values, ReifyMathExpressions(root->Children));
    return ConvertToExceptionOrNumericValue(CSSMathProduct::Create(krys::move(values)));
  }

  KRYS_NODISCARD static ExceptionOr<Ref<CSSNumericValue>>
    ReifyMathExpression(const css::calc::IndirectNode<css::calc::Negate> &root) noexcept
  {
    KRYS_CSS_NUMERIC_RETURN_IF_EXCEPTION(child, CSSNumericValue::ReifyMathExpression(root->A));
    return ConvertToExceptionOrNumericValue(CSSMathNegate::Create(krys::move(child)));
  }

  KRYS_NODISCARD static ExceptionOr<Ref<CSSNumericValue>>
    ReifyMathExpression(const css::calc::IndirectNode<css::calc::Invert> &root) noexcept
  {
    KRYS_CSS_NUMERIC_RETURN_IF_EXCEPTION(child, CSSNumericValue::ReifyMathExpression(root->A));
    return ConvertToExceptionOrNumericValue(CSSMathInvert::Create(krys::move(child)));
  }

  KRYS_NODISCARD static ExceptionOr<Ref<CSSNumericValue>>
    ReifyMathExpression(const css::calc::IndirectNode<css::calc::Min> &root) noexcept
  {
    KRYS_CSS_NUMERIC_RETURN_IF_EXCEPTION(values, ReifyMathExpressions(root->Children));
    return ConvertToExceptionOrNumericValue(CSSMathMin::Create(krys::move(values)));
  }

  KRYS_NODISCARD static ExceptionOr<Ref<CSSNumericValue>>
    ReifyMathExpression(const css::calc::IndirectNode<css::calc::Max> &root) noexcept
  {
    KRYS_CSS_NUMERIC_RETURN_IF_EXCEPTION(values, ReifyMathExpressions(root->Children));
    return ConvertToExceptionOrNumericValue(CSSMathMax::Create(krys::move(values)));
  }

  KRYS_NODISCARD static ExceptionOr<Ref<CSSNumericValue>>
    ReifyMathExpression(const css::calc::IndirectNode<css::calc::Clamp> &root) noexcept
  {
    KRYS_CSS_NUMERIC_RETURN_IF_EXCEPTION(min, CSSNumericValue::ReifyMathExpression(root->Min));
    KRYS_CSS_NUMERIC_RETURN_IF_EXCEPTION(val, CSSNumericValue::ReifyMathExpression(root->Val));
    KRYS_CSS_NUMERIC_RETURN_IF_EXCEPTION(max, CSSNumericValue::ReifyMathExpression(root->Max));
    return ConvertToExceptionOrNumericValue(
      CSSMathClamp::Create(krys::move(min), krys::move(val), krys::move(max)));
  }

  template <typename TOp>
  KRYS_NODISCARD static ExceptionOr<Ref<CSSNumericValue>>
    ReifyMathExpression(const css::calc::IndirectNode<TOp> &) noexcept
  {
    return Exception {ExceptionCode::UnknownError};
  }

  ExceptionOr<Ref<CSSNumericValue>> CSSNumericValue::ReifyMathExpression(const css::calc::Tree &tree) noexcept
  {
    return CSSNumericValue::ReifyMathExpression(tree.Root);
  }

  ExceptionOr<Ref<CSSNumericValue>>
    CSSNumericValue::ReifyMathExpression(const css::calc::Child &root) noexcept
  {
    return root.SwitchOn([](const auto &child) { return web::ReifyMathExpression(child); });
  }

  ExceptionOr<Ref<CSSNumericValue>>
    CSSNumericValue::ReifyMathExpression(const css::calc::ChildOrNone &root) noexcept
  {
    return root.SwitchOn([](const css::calc::Child &child) -> ExceptionOr<Ref<CSSNumericValue>>
                         { return CSSNumericValue::ReifyMathExpression(child); },
                         [](const css::keywords::None &) -> ExceptionOr<Ref<CSSNumericValue>>
                         { return Exception {ExceptionCode::UnknownError}; });
  }

  KRYS_NODISCARD static Ref<CSSNumericValue> Negate(Ref<CSSNumericValue> &&value) noexcept
  {
    // https://drafts.css-houdini.org/css-typed-om/#cssmath-negate-a-cssnumericvalue
    if (auto *mathNegate = DynamicDowncast<CSSMathNegate>(value.get()))
    {
      return ShareRef(mathNegate->Value());
    }
    if (auto *unitValue = DynamicDowncast<CSSUnitValue>(value.get()))
    {
      return CSSUnitValue::Create(-unitValue->Value(), unitValue->UnitEnum());
    }
    return CSSMathNegate::Create(krys::move(value));
  }

  KRYS_NODISCARD static ExceptionOr<Ref<CSSNumericValue>> Invert(Ref<CSSNumericValue> &&value) noexcept
  {
    // https://drafts.css-houdini.org/css-typed-om/#cssmath-invert-a-cssnumericvalue
    if (auto *mathInvert = DynamicDowncast<CSSMathInvert>(value.get()))
    {
      return ShareRef(mathInvert->Value());
    }

    if (auto *unitValue = DynamicDowncast<CSSUnitValue>(value.get()))
    {
      if (unitValue->UnitEnum() == css::UnitType::Number)
      {
        if (unitValue->Value() == 0.0 || unitValue->Value() == -0.0)
        {
          return Exception {ExceptionCode::RangeError};
        }
        return Ref<CSSNumericValue> {CSSUnitValue::Create(1.0 / unitValue->Value(), unitValue->UnitEnum())};
      }
    }

    return Ref<CSSNumericValue> {CSSMathInvert::Create(krys::move(value))};
  }

  template <typename T>
  KRYS_NODISCARD static RefPtr<CSSNumericValue>
    OperationOnValuesOfSameUnit(T operation, const List<Ref<CSSNumericValue>> &values) noexcept
  {
    bool allValuesHaveSameUnit =
      !values.empty()
      && std::ranges::all_of(
        values,
        [&](const auto &value)
        {
          auto *unitValue = DynamicDowncast<CSSUnitValue>(value.get());
          return unitValue ? unitValue->UnitEnum() == Downcast<CSSUnitValue>(*values[0]).UnitEnum() : false;
        });
    if (allValuesHaveSameUnit)
    {
      const auto &firstUnitValue = Downcast<CSSUnitValue>(*values[0]);
      auto unit = firstUnitValue.UnitEnum();
      double result = firstUnitValue.Value();
      for (size_t i = 1; i < values.size(); ++i)
      {
        result = operation(result, Downcast<CSSUnitValue>(*values[i]).Value());
      }
      return CSSUnitValue::Create(result, unit);
    }
    return nullptr;
  }

  KRYS_NODISCARD static List<Ref<CSSNumericValue>> RectifyNumberishes(List<CSSNumberish> numberishes) noexcept
  {
    List<Ref<CSSNumericValue>> values;
    values.reserve(numberishes.size());
    for (auto &numberish : numberishes)
    {
      values.push_back(CSSNumericValue::RectifyNumberish(krys::move(numberish)));
    }
    return values;
  }

  template <typename T>
  List<Ref<CSSNumericValue>>
    CSSNumericValue::PrependItemsOfTypeOrThis(List<Ref<CSSNumericValue>> numericValues) noexcept
  {
    List<Ref<CSSNumericValue>> values;
    if (T *t = DynamicDowncast<T>(*this))
    {
      const auto &array = t->Values().Array();
      values.insert(values.end(), array.begin(), array.end());
    }
    else
    {
      values.push_back(ShareRef(*this));
    }
    values.insert(values.end(), std::make_move_iterator(numericValues.begin()),
                  std::make_move_iterator(numericValues.end()));
    return values;
  }

  ExceptionOr<Ref<CSSNumericValue>>
    CSSNumericValue::AddInternal(List<Ref<CSSNumericValue>> numericValues) noexcept
  {
    // https://drafts.css-houdini.org/css-typed-om/#dom-cssnumericvalue-add
    auto values = PrependItemsOfTypeOrThis<CSSMathSum>(krys::move(numericValues));

    if (auto result = OperationOnValuesOfSameUnit(std::plus<>(), values))
    {
      return Ref<CSSNumericValue> {krys::move(result)};
    }

    return ConvertToExceptionOrNumericValue(CSSMathSum::Create(krys::move(values)));
  }

  ExceptionOr<Ref<CSSNumericValue>> CSSNumericValue::Add(List<CSSNumberish> values) noexcept
  {
    return AddInternal(RectifyNumberishes(krys::move(values)));
  }

  ExceptionOr<Ref<CSSNumericValue>> CSSNumericValue::Sub(List<CSSNumberish> values) noexcept
  {
    List<Ref<CSSNumericValue>> negatedValues;
    negatedValues.reserve(values.size());
    for (auto &numberish : values)
    {
      negatedValues.push_back(web::Negate(RectifyNumberish(krys::move(numberish))));
    }
    return AddInternal(krys::move(negatedValues));
  }

  ExceptionOr<Ref<CSSNumericValue>>
    CSSNumericValue::MultiplyInternal(List<Ref<CSSNumericValue>> numericValues) noexcept
  {
    // https://drafts.css-houdini.org/css-typed-om/#dom-cssnumericvalue-mul
    auto values = PrependItemsOfTypeOrThis<CSSMathProduct>(krys::move(numericValues));

    bool allUnitValues =
      std::ranges::all_of(values, [](const auto &value) { return Is<CSSUnitValue>(value.get()); });
    if (allUnitValues)
    {
      bool multipleUnitsFound {false};
      Maybe<size_t> nonNumberUnitIndex;
      for (size_t i = 0; i < values.size(); ++i)
      {
        auto unit = Downcast<CSSUnitValue>(*values[i]).UnitEnum();
        if (unit == css::UnitType::Number)
        {
          continue;
        }
        if (nonNumberUnitIndex)
        {
          multipleUnitsFound = true;
          break;
        }
        nonNumberUnitIndex = i;
      }
      if (!multipleUnitsFound)
      {
        double product = 1;
        for (const Ref<CSSNumericValue> &value : values)
        {
          product *= Downcast<CSSUnitValue>(*value).Value();
        }
        auto unit = nonNumberUnitIndex ? Downcast<CSSUnitValue>(*values[*nonNumberUnitIndex]).UnitEnum()
                                       : css::UnitType::Number;
        return Ref<CSSNumericValue> {CSSUnitValue::Create(product, unit)};
      }
    }

    return ConvertToExceptionOrNumericValue(CSSMathProduct::Create(krys::move(values)));
  }

  ExceptionOr<Ref<CSSNumericValue>> CSSNumericValue::Mul(List<CSSNumberish> values) noexcept
  {
    return MultiplyInternal(RectifyNumberishes(krys::move(values)));
  }

  ExceptionOr<Ref<CSSNumericValue>> CSSNumericValue::Div(List<CSSNumberish> values) noexcept
  {
    List<Ref<CSSNumericValue>> invertedValues;
    invertedValues.reserve(values.size());
    for (auto &value : values)
    {
      KRYS_CSS_NUMERIC_RETURN_IF_EXCEPTION(inverted, web::Invert(RectifyNumberish(krys::move(value))));
      invertedValues.push_back(krys::move(inverted));
    }
    return MultiplyInternal(krys::move(invertedValues));
  }

  ExceptionOr<Ref<CSSNumericValue>> CSSNumericValue::Min(List<CSSNumberish> numberishes) noexcept
  {
    // https://drafts.css-houdini.org/css-typed-om/#dom-cssnumericvalue-min
    auto values = PrependItemsOfTypeOrThis<CSSMathMin>(RectifyNumberishes(krys::move(numberishes)));

    if (auto result = OperationOnValuesOfSameUnit([](double a, double b) { return std::min(a, b); }, values))
    {
      return Ref<CSSNumericValue> {krys::move(result)};
    }

    return ConvertToExceptionOrNumericValue(CSSMathMin::Create(krys::move(values)));
  }

  ExceptionOr<Ref<CSSNumericValue>> CSSNumericValue::Max(List<CSSNumberish> numberishes) noexcept
  {
    // https://drafts.css-houdini.org/css-typed-om/#dom-cssnumericvalue-max
    auto values = PrependItemsOfTypeOrThis<CSSMathMax>(RectifyNumberishes(krys::move(numberishes)));

    if (auto result = OperationOnValuesOfSameUnit([](double a, double b) { return std::max(a, b); }, values))
    {
      return Ref<CSSNumericValue> {krys::move(result)};
    }

    return ConvertToExceptionOrNumericValue(CSSMathMax::Create(krys::move(values)));
  }

  Ref<CSSNumericValue> CSSNumericValue::RectifyNumberish(CSSNumberish numberish) noexcept
  {
    // https://drafts.css-houdini.org/css-typed-om/#rectify-a-numberish-value
    return krys::SwitchOn(
      numberish,
      [](RefPtr<CSSNumericValue> &value) -> Ref<CSSNumericValue>
      {
        krys_assert(value);
        return krys::move(value);
      },
      [](double value) -> Ref<CSSNumericValue> { return CSSNumericFactory::Number(value); });
  }

  bool CSSNumericValue::Equals(List<CSSNumberish> values) noexcept
  {
    // https://drafts.css-houdini.org/css-typed-om/#dom-cssnumericvalue-equals
    auto numericValues = RectifyNumberishes(krys::move(values));
    return std::ranges::all_of(numericValues, [&](const auto &value) { return this->Equals(*value); });
  }

  ExceptionOr<Ref<CSSUnitValue>> CSSNumericValue::To(const CSSOMString &unit) noexcept
  {
    return To(CSSUnitValue::ParseUnit(unit));
  }

  // https://drafts.css-houdini.org/css-typed-om/#create-a-cssunitvalue-from-a-sum-value-item
  KRYS_NODISCARD static RefPtr<CSSUnitValue>
    CreateCSSUnitValueFromAddend(const CSSNumericValue::Addend &addend) noexcept
  {
    if (addend.Units.size() > 1)
    {
      return nullptr;
    }
    if (addend.Units.empty())
    {
      return CSSUnitValue::Create(addend.Value, css::UnitType::Number);
    }
    auto unit = addend.Units.begin();
    if (unit->second != 1)
    {
      return nullptr;
    }
    return CSSUnitValue::Create(addend.Value, unit->first);
  }

  ExceptionOr<Ref<CSSUnitValue>> CSSNumericValue::To(css::UnitType unit) noexcept
  {
    // https://drafts.css-houdini.org/css-typed-om/#dom-cssnumericvalue-to
    auto type = CSSNumericType::Create(unit);
    if (!type)
    {
      return Exception {ExceptionCode::SyntaxError};
    }

    auto sumValue = ToSumValue();
    if (!sumValue || sumValue->size() != 1)
    {
      return Exception {ExceptionCode::TypeError};
    }

    const auto &addend = (*sumValue)[0];
    auto unconverted = CreateCSSUnitValueFromAddend(addend);
    if (!unconverted)
    {
      return Exception {ExceptionCode::TypeError};
    }

    auto converted = unconverted->ConvertTo(unit);
    if (!converted)
    {
      return Exception {ExceptionCode::TypeError};
    }
    return Ref<CSSUnitValue> {krys::move(converted)};
  }

  ExceptionOr<Ref<CSSMathSum>> CSSNumericValue::ToSum(const List<CSSOMString> &units) noexcept
  {
    // https://drafts.css-houdini.org/css-typed-om/#dom-cssnumericvalue-tosum
    List<css::UnitType> parsedUnits;
    parsedUnits.reserve(units.size());
    for (const auto &unit : units)
    {
      auto parsedUnit = CSSUnitValue::ParseUnit(unit);
      if (parsedUnit == css::UnitType::Unknown)
      {
        return Exception {ExceptionCode::SyntaxError, u8"Invalid unit parameter"};
      }
      parsedUnits.push_back(parsedUnit);
    }
    auto sumValue = ToSumValue();
    if (!sumValue)
    {
      return Exception {ExceptionCode::TypeError, u8"Could not create a sum value"};
    }

    List<Ref<CSSNumericValue>> values;
    values.reserve(sumValue->size());
    for (const auto &addend : *sumValue)
    {
      auto cssUnitValue = CreateCSSUnitValueFromAddend(addend);
      if (!cssUnitValue)
      {
        return Exception {ExceptionCode::TypeError, u8"Could not create CSSUnitValue"};
      }
      values.emplace_back(krys::move(cssUnitValue));
    }

    if (parsedUnits.empty())
    {
      std::ranges::sort(values,
                        [](const auto &a, const auto &b)
                        {
                          return Downcast<CSSUnitValue>(*a).UnitSerialisation()
                                 < Downcast<CSSUnitValue>(*b).UnitSerialisation();
                        });
      return CSSMathSum::Create(krys::move(values));
    }

    List<Ref<CSSNumericValue>> result;
    for (auto parsedUnit : parsedUnits)
    {
      auto temp = CSSUnitValue::Create(0, parsedUnit);
      for (size_t i = 0; i < values.size();)
      {
        const auto &value = Downcast<CSSUnitValue>(*values[i]);
        if (auto convertedValue = value.ConvertTo(parsedUnit))
        {
          temp->SetValue(temp->Value() + convertedValue->Value());
          values.erase(values.begin() + static_cast<ptrdiff_t>(i));
        }
        else
        {
          ++i;
        }
      }
      result.emplace_back(krys::move(temp));
    }

    if (!values.empty())
    {
      return Exception {ExceptionCode::TypeError, u8"Failed to convert all values"};
    }

    return CSSMathSum::Create(krys::move(result));
  }

  KRYS_NODISCARD static css::UnitType TokenUnitType(const css::Token &token) noexcept
  {
    switch (token.Type())
    {
      case css::TokenType::Number:     return css::UnitType::Number;
      case css::TokenType::Percentage: return css::UnitType::Percentage;
      case css::TokenType::Dimension:  return css::ParseUnitType(token.Unit());
      default:                         return css::UnitType::Unknown;
    }
  }

  ExceptionOr<Ref<CSSNumericValue>> CSSNumericValue::Parse(Document &document, CSSOMString &&cssText) noexcept
  {
    // https://drafts.css-houdini.org/css-typed-om/#dom-cssnumericvalue-parse
    css::InputStream inputStream {krys::move(cssText)};
    css::Tokenizer tokenizer(inputStream);
    std::ignore = tokenizer.PumpTokenizer();
    auto range = tokenizer.Tokens();
    range.DiscardWhitespace();
    if (range.IsAtEnd())
    {
      return Exception {ExceptionCode::SyntaxError, u8"Failed to parse CSS text"};
    }
    auto componentValueStart = range;
    range.DiscardComponentValue();
    auto componentValueEnd = range;
    range.DiscardWhitespace();
    if (!range.IsAtEnd())
    {
      return Exception {ExceptionCode::SyntaxError, u8"Failed to parse CSS text"};
    }

    auto componentValueRange = componentValueStart.RangeUntil(componentValueEnd);
    // https://drafts.css-houdini.org/css-typed-om/#reify-a-numeric-value
    switch (componentValueRange.Peek().Type())
    {
      case css::TokenType::Dimension:
      case css::TokenType::Number:
      case css::TokenType::Percentage:
      {
        const auto &token = componentValueRange.ConsumeIncludingWhitespace();
        auto unitType = TokenUnitType(token);
        if (token.Type() == css::TokenType::Dimension && !CSSNumericType::Create(unitType))
        {
          return Exception {ExceptionCode::SyntaxError, u8"Failed to parse CSS text"};
        }
        return ConvertToExceptionOrNumericValue(CSSUnitValue::Create(token.NumericValue(), unitType));
      }
      case css::TokenType::Function:
      {
        auto functionId = componentValueRange.Peek().FunctionId();
        if (functionId == css::ValueId::Calc || functionId == css::ValueId::Min
            || functionId == css::ValueId::Max || functionId == css::ValueId::Clamp)
        {
          // FIXME: The spec is unclear on what context to use when parsing in CSSNumericValue so for the
          // time-being, we use `Category::LengthPercentage`, as it is the most permissive.
          // See https://github.com/w3c/csswg-drafts/issues/10753

          auto parserContext = css::ParserContext {document};
          auto parserState = css::PropertyParserState {
            .Context = parserContext,
          };
          auto parserOptions = css::calc::ParserOptions {
            .Category = css::Category::LengthPercentage,
            .Range = css::All,
            .AllowedSymbols = {},
            .PropertyOptions = {},
          };
          auto simplificationOptions = css::calc::SimplificationOptions {
            .Category = css::Category::LengthPercentage,
            .Range = css::All,
            .ConversionData = null,
            .SymbolTable = {},
            .AllowZeroValueLengthRemovalFromSum = false,
          };
          auto tree = css::calc::ParseAndSimplify(componentValueRange, parserState, parserOptions,
                                                  simplificationOptions);
          if (!tree)
          {
            return Exception {ExceptionCode::SyntaxError, u8"Failed to parse CSS text"};
          }

          return ReifyMathExpression(*tree);
        }
        break;
      }
      default:
      {
        break;
      }
    }
    return Exception {ExceptionCode::SyntaxError, u8"Failed to parse CSS text"};
  }
}

#undef KRYS_CSS_NUMERIC_RETURN_IF_EXCEPTION
