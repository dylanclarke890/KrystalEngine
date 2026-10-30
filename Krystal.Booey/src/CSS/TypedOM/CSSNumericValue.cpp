#include "Krystal.Booey/CSS/TypedOM/CSSNumericValue.hpp"
#include "Krystal.Booey/CSS/Calc/SymbolTable.hpp"
#include "Krystal.Booey/CSS/Calc/Tree.hpp"
#include "Krystal.Booey/CSS/Calc/TreeParser.hpp"
#include "Krystal.Booey/CSS/Calc/TreeSimplification.hpp"
#include "Krystal.Booey/CSS/Parser/InputStream.hpp"
#include "Krystal.Booey/CSS/Parser/ParserContext.hpp"
#include "Krystal.Booey/CSS/Parser/Tokenizer.hpp"
#include "Krystal.Booey/CSS/Parser/TokenRange.hpp"
#include "Krystal.Booey/CSS/Properties/PropertyParserState.hpp"
#include "Krystal.Booey/CSS/TypedOM/CSSNumericFactory.hpp"
#include "Krystal.Booey/CSS/TypedOM/CSSUnitValue.hpp"
#include "Krystal.Booey/CSS/TypedOM/Numeric/CSSMathClamp.hpp"
#include "Krystal.Booey/CSS/TypedOM/Numeric/CSSMathInvert.hpp"
#include "Krystal.Booey/CSS/TypedOM/Numeric/CSSMathMax.hpp"
#include "Krystal.Booey/CSS/TypedOM/Numeric/CSSMathMin.hpp"
#include "Krystal.Booey/CSS/TypedOM/Numeric/CSSMathNegate.hpp"
#include "Krystal.Booey/CSS/TypedOM/Numeric/CSSMathProduct.hpp"
#include "Krystal.Booey/CSS/TypedOM/Numeric/CSSMathSum.hpp"
#include "Krystal.Booey/CSS/TypedOM/Numeric/CSSNumericArray.hpp"
#include <algorithm>
#include <functional>

// Explicitly prefixed name used to avoid conflicts with existing macros that can be indirectly included.
#define CSS_NUMERIC_RETURN_IF_EXCEPTION(resultVariable, expression)                                          \
  auto resultOrException_##resultVariable = expression;                                                      \
  if (resultOrException_##resultVariable.HasException())                                                     \
  {                                                                                                          \
    return resultOrException_##resultVariable.ReleaseException();                                            \
  }                                                                                                          \
  auto resultVariable = resultOrException_##resultVariable.ReleaseValue()

namespace krys::boo::css
{
  template <typename T>
  KRYS_NODISCARD static dom::ExceptionOr<Ref<CSSNumericValue>>
    ConvertToExceptionOrNumericValue(dom::ExceptionOr<Ref<T>> &&input) noexcept
  {
    CSS_NUMERIC_RETURN_IF_EXCEPTION(result, krys::move(input));
    return Ref<CSSNumericValue> {krys::move(result)};
  }

  template <typename T>
  KRYS_NODISCARD static dom::ExceptionOr<Ref<CSSNumericValue>>
    ConvertToExceptionOrNumericValue(Ref<T> &&input) noexcept
  {
    return Ref<CSSNumericValue> {krys::move(input)};
  }

  KRYS_NODISCARD static dom::ExceptionOr<List<Ref<CSSNumericValue>>>
    ReifyMathExpressions(const calc::Children &nodes) noexcept
  {
    List<Ref<CSSNumericValue>> values;
    values.reserve(nodes.size());
    for (const auto &node : nodes)
    {
      CSS_NUMERIC_RETURN_IF_EXCEPTION(value, CSSNumericValue::ReifyMathExpression(node));
      values.push_back(krys::move(value));
    }
    return values;
  }

  template <calc::Numeric T>
  KRYS_NODISCARD static dom::ExceptionOr<Ref<CSSNumericValue>> ReifyMathExpression(const T &root) noexcept
  {
    return ConvertToExceptionOrNumericValue(CSSUnitValue::Create(root.Value, calc::ToUnit(root)));
  }

  KRYS_NODISCARD static dom::ExceptionOr<Ref<CSSNumericValue>>
    ReifyMathExpression(const calc::Symbol &) noexcept
  {
    // CSS Typed OM doesn't currently support unresolved symbols.
    return dom::Exception {dom::ExceptionCode::UnknownError};
  }

  KRYS_NODISCARD static dom::ExceptionOr<Ref<CSSNumericValue>>
    ReifyMathExpression(const calc::SiblingCount &) noexcept
  {
    // CSS Typed OM doesn't currently support unresolved sibling-count() functions.
    return dom::Exception {dom::ExceptionCode::UnknownError};
  }

  KRYS_NODISCARD static dom::ExceptionOr<Ref<CSSNumericValue>>
    ReifyMathExpression(const calc::SiblingIndex &) noexcept
  {
    // CSS Typed OM doesn't currently support unresolved sibling-index() functions.
    return dom::Exception {dom::ExceptionCode::UnknownError};
  }

  KRYS_NODISCARD static dom::ExceptionOr<Ref<CSSNumericValue>>
    ReifyMathExpression(const calc::IndirectNode<calc::Sum> &root) noexcept
  {
    CSS_NUMERIC_RETURN_IF_EXCEPTION(values, ReifyMathExpressions(root->Children));
    return ConvertToExceptionOrNumericValue(CSSMathSum::Create(krys::move(values)));
  }

  KRYS_NODISCARD static dom::ExceptionOr<Ref<CSSNumericValue>>
    ReifyMathExpression(const calc::IndirectNode<calc::Product> &root) noexcept
  {
    CSS_NUMERIC_RETURN_IF_EXCEPTION(values, ReifyMathExpressions(root->Children));
    return ConvertToExceptionOrNumericValue(CSSMathProduct::Create(krys::move(values)));
  }

  KRYS_NODISCARD static dom::ExceptionOr<Ref<CSSNumericValue>>
    ReifyMathExpression(const calc::IndirectNode<calc::Negate> &root) noexcept
  {
    CSS_NUMERIC_RETURN_IF_EXCEPTION(child, CSSNumericValue::ReifyMathExpression(root->A));
    return ConvertToExceptionOrNumericValue(
      CSSMathNegate::Create(RefPtr<CSSNumericValue> {krys::move(child)}));
  }

  KRYS_NODISCARD static dom::ExceptionOr<Ref<CSSNumericValue>>
    ReifyMathExpression(const calc::IndirectNode<calc::Invert> &root) noexcept
  {
    CSS_NUMERIC_RETURN_IF_EXCEPTION(child, CSSNumericValue::ReifyMathExpression(root->A));
    return ConvertToExceptionOrNumericValue(
      CSSMathInvert::Create(RefPtr<CSSNumericValue> {krys::move(child)}));
  }

  KRYS_NODISCARD static dom::ExceptionOr<Ref<CSSNumericValue>>
    ReifyMathExpression(const calc::IndirectNode<calc::Min> &root) noexcept
  {
    CSS_NUMERIC_RETURN_IF_EXCEPTION(values, ReifyMathExpressions(root->Children));
    return ConvertToExceptionOrNumericValue(CSSMathMin::Create(krys::move(values)));
  }

  KRYS_NODISCARD static dom::ExceptionOr<Ref<CSSNumericValue>>
    ReifyMathExpression(const calc::IndirectNode<calc::Max> &root) noexcept
  {
    CSS_NUMERIC_RETURN_IF_EXCEPTION(values, ReifyMathExpressions(root->Children));
    return ConvertToExceptionOrNumericValue(CSSMathMax::Create(krys::move(values)));
  }

  KRYS_NODISCARD static dom::ExceptionOr<Ref<CSSNumericValue>>
    ReifyMathExpression(const calc::IndirectNode<calc::Clamp> &root) noexcept
  {
    CSS_NUMERIC_RETURN_IF_EXCEPTION(min, CSSNumericValue::ReifyMathExpression(root->Min));
    CSS_NUMERIC_RETURN_IF_EXCEPTION(val, CSSNumericValue::ReifyMathExpression(root->Val));
    CSS_NUMERIC_RETURN_IF_EXCEPTION(max, CSSNumericValue::ReifyMathExpression(root->Max));
    return ConvertToExceptionOrNumericValue(CSSMathClamp::Create(RefPtr<CSSNumericValue> {krys::move(min)},
                                                                 RefPtr<CSSNumericValue> {krys::move(val)},
                                                                 RefPtr<CSSNumericValue> {krys::move(max)}));
  }

  template <typename TOp>
  KRYS_NODISCARD static dom::ExceptionOr<Ref<CSSNumericValue>>
    ReifyMathExpression(const calc::IndirectNode<TOp> &) noexcept
  {
    return dom::Exception {dom::ExceptionCode::UnknownError};
  }

  dom::ExceptionOr<Ref<CSSNumericValue>> CSSNumericValue::ReifyMathExpression(const calc::Tree &tree) noexcept
  {
    return CSSNumericValue::ReifyMathExpression(tree.Root);
  }

  dom::ExceptionOr<Ref<CSSNumericValue>>
    CSSNumericValue::ReifyMathExpression(const calc::Child &root) noexcept
  {
    return root.SwitchOn([](const auto &child) { return css::ReifyMathExpression(child); });
  }

  dom::ExceptionOr<Ref<CSSNumericValue>>
    CSSNumericValue::ReifyMathExpression(const calc::ChildOrNone &root) noexcept
  {
    return root.SwitchOn([](const calc::Child &child) -> dom::ExceptionOr<Ref<CSSNumericValue>>
                         { return CSSNumericValue::ReifyMathExpression(child); },
                         [](const keywords::None &) -> dom::ExceptionOr<Ref<CSSNumericValue>>
                         { return dom::Exception {dom::ExceptionCode::UnknownError}; });
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
    return CSSMathNegate::Create(RefPtr<CSSNumericValue> {krys::move(value)});
  }

  KRYS_NODISCARD static dom::ExceptionOr<Ref<CSSNumericValue>> Invert(Ref<CSSNumericValue> &&value) noexcept
  {
    // https://drafts.css-houdini.org/css-typed-om/#cssmath-invert-a-cssnumericvalue
    if (auto *mathInvert = DynamicDowncast<CSSMathInvert>(value.get()))
    {
      return ShareRef(mathInvert->Value());
    }

    if (auto *unitValue = DynamicDowncast<CSSUnitValue>(value.get()))
    {
      if (unitValue->UnitEnum() == UnitType::Number)
      {
        if (unitValue->Value() == 0.0 || unitValue->Value() == -0.0)
        {
          return dom::Exception {dom::ExceptionCode::RangeError};
        }
        return Ref<CSSNumericValue> {CSSUnitValue::Create(1.0 / unitValue->Value(), unitValue->UnitEnum())};
      }
    }

    return Ref<CSSNumericValue> {CSSMathInvert::Create(RefPtr<CSSNumericValue> {krys::move(value)})};
  }

  template <typename T>
  KRYS_NODISCARD static RefPtr<CSSNumericValue>
    OperationOnValuesOfSameUnit(T &&operation, const List<Ref<CSSNumericValue>> &values) noexcept
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

  KRYS_NODISCARD static List<Ref<CSSNumericValue>>
    RectifyNumberishes(List<CSSNumberish> &&numberishes) noexcept
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
    CSSNumericValue::PrependItemsOfTypeOrThis(List<Ref<CSSNumericValue>> &&numericValues) noexcept
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

  dom::ExceptionOr<Ref<CSSNumericValue>>
    CSSNumericValue::AddInternal(List<Ref<CSSNumericValue>> &&numericValues) noexcept
  {
    // https://drafts.css-houdini.org/css-typed-om/#dom-cssnumericvalue-add
    auto values = PrependItemsOfTypeOrThis<CSSMathSum>(krys::move(numericValues));

    if (auto result = OperationOnValuesOfSameUnit(std::plus<double>(), values))
    {
      return Ref<CSSNumericValue> {krys::move(result)};
    }

    return ConvertToExceptionOrNumericValue(CSSMathSum::Create(krys::move(values)));
  }

  dom::ExceptionOr<Ref<CSSNumericValue>> CSSNumericValue::Add(List<CSSNumberish> &&values) noexcept
  {
    return AddInternal(RectifyNumberishes(krys::move(values)));
  }

  dom::ExceptionOr<Ref<CSSNumericValue>> CSSNumericValue::Sub(List<CSSNumberish> &&values) noexcept
  {
    List<Ref<CSSNumericValue>> negatedValues;
    negatedValues.reserve(values.size());
    for (auto &numberish : values)
    {
      negatedValues.push_back(css::Negate(RectifyNumberish(krys::move(numberish))));
    }
    return AddInternal(krys::move(negatedValues));
  }

  dom::ExceptionOr<Ref<CSSNumericValue>>
    CSSNumericValue::MultiplyInternal(List<Ref<CSSNumericValue>> &&numericValues) noexcept
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
        if (unit == UnitType::Number)
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
                                       : UnitType::Number;
        return Ref<CSSNumericValue> {CSSUnitValue::Create(product, unit)};
      }
    }

    return ConvertToExceptionOrNumericValue(CSSMathProduct::Create(krys::move(values)));
  }

  dom::ExceptionOr<Ref<CSSNumericValue>> CSSNumericValue::Mul(List<CSSNumberish> &&values) noexcept
  {
    return MultiplyInternal(RectifyNumberishes(krys::move(values)));
  }

  dom::ExceptionOr<Ref<CSSNumericValue>> CSSNumericValue::Div(List<CSSNumberish> &&values) noexcept
  {
    List<Ref<CSSNumericValue>> invertedValues;
    invertedValues.reserve(values.size());
    for (auto &value : values)
    {
      CSS_NUMERIC_RETURN_IF_EXCEPTION(inverted, css::Invert(RectifyNumberish(krys::move(value))));
      invertedValues.push_back(krys::move(inverted));
    }
    return MultiplyInternal(krys::move(invertedValues));
  }

  dom::ExceptionOr<Ref<CSSNumericValue>> CSSNumericValue::Min(List<CSSNumberish> &&numberishes) noexcept
  {
    // https://drafts.css-houdini.org/css-typed-om/#dom-cssnumericvalue-min
    auto values = PrependItemsOfTypeOrThis<CSSMathMin>(RectifyNumberishes(krys::move(numberishes)));

    if (auto result = OperationOnValuesOfSameUnit([](double a, double b) { return std::min(a, b); }, values))
    {
      return Ref<CSSNumericValue> {krys::move(result)};
    }

    return ConvertToExceptionOrNumericValue(CSSMathMin::Create(krys::move(values)));
  }

  dom::ExceptionOr<Ref<CSSNumericValue>> CSSNumericValue::Max(List<CSSNumberish> &&numberishes) noexcept
  {
    // https://drafts.css-houdini.org/css-typed-om/#dom-cssnumericvalue-max
    auto values = PrependItemsOfTypeOrThis<CSSMathMax>(RectifyNumberishes(krys::move(numberishes)));

    if (auto result = OperationOnValuesOfSameUnit([](double a, double b) { return std::max(a, b); }, values))
    {
      return Ref<CSSNumericValue> {krys::move(result)};
    }

    return ConvertToExceptionOrNumericValue(CSSMathMax::Create(krys::move(values)));
  }

  Ref<CSSNumericValue> CSSNumericValue::RectifyNumberish(CSSNumberish &&numberish) noexcept
  {
    // https://drafts.css-houdini.org/css-typed-om/#rectify-a-numberish-value
    return krys::SwitchOn(
      numberish,
      [](RefPtr<CSSNumericValue> &value)
      {
        krys_assert(value);
        return Ref<CSSNumericValue> {krys::move(value)};
      },
      [](double value) { return Ref<CSSNumericValue> {CSSNumericFactory::Number(value)}; });
  }

  bool CSSNumericValue::Equals(List<CSSNumberish> &&values) noexcept
  {
    // https://drafts.css-houdini.org/css-typed-om/#dom-cssnumericvalue-equals
    auto numericValues = RectifyNumberishes(krys::move(values));
    return std::ranges::all_of(numericValues, [&](const auto &value) { return this->Equals(*value); });
  }

  dom::ExceptionOr<Ref<CSSUnitValue>> CSSNumericValue::To(const CSSOMString &unit) noexcept
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
      return CSSUnitValue::Create(addend.Value, UnitType::Number);
    }
    auto unit = addend.Units.begin();
    if (unit->second != 1)
    {
      return nullptr;
    }
    return CSSUnitValue::Create(addend.Value, unit->first);
  }

  dom::ExceptionOr<Ref<CSSUnitValue>> CSSNumericValue::To(UnitType unit) noexcept
  {
    // https://drafts.css-houdini.org/css-typed-om/#dom-cssnumericvalue-to
    auto type = CSSNumericType::Create(unit);
    if (!type)
    {
      return dom::Exception {dom::ExceptionCode::SyntaxError};
    }

    auto sumValue = ToSumValue();
    if (!sumValue || sumValue->size() != 1)
    {
      return dom::Exception {dom::ExceptionCode::TypeError};
    }

    const auto &addend = (*sumValue)[0];
    auto unconverted = CreateCSSUnitValueFromAddend(addend);
    if (!unconverted)
    {
      return dom::Exception {dom::ExceptionCode::TypeError};
    }

    auto converted = unconverted->ConvertTo(unit);
    if (!converted)
    {
      return dom::Exception {dom::ExceptionCode::TypeError};
    }
    return Ref<CSSUnitValue> {krys::move(converted)};
  }

  dom::ExceptionOr<Ref<CSSMathSum>> CSSNumericValue::ToSum(List<CSSOMString> &&units) noexcept
  {
    // https://drafts.css-houdini.org/css-typed-om/#dom-cssnumericvalue-tosum
    List<UnitType> parsedUnits;
    parsedUnits.reserve(units.size());
    for (const auto &unit : units)
    {
      auto parsedUnit = CSSUnitValue::ParseUnit(unit);
      if (parsedUnit == UnitType::Unknown)
      {
        return dom::Exception {dom::ExceptionCode::SyntaxError, u8"Invalid unit parameter"};
      }
      parsedUnits.push_back(parsedUnit);
    }
    auto sumValue = ToSumValue();
    if (!sumValue)
    {
      return dom::Exception {dom::ExceptionCode::TypeError, u8"Could not create a sum value"};
    }

    List<Ref<CSSNumericValue>> values;
    values.reserve(sumValue->size());
    for (const auto &addend : *sumValue)
    {
      auto cssUnitValue = CreateCSSUnitValueFromAddend(addend);
      if (!cssUnitValue)
      {
        return dom::Exception {dom::ExceptionCode::TypeError, u8"Could not create CSSUnitValue"};
      }
      values.push_back(Ref<CSSNumericValue> {krys::move(cssUnitValue)});
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
      result.push_back(krys::move(temp));
    }

    if (!values.empty())
    {
      return dom::Exception {dom::ExceptionCode::TypeError, u8"Failed to convert all values"};
    }

    return CSSMathSum::Create(krys::move(result));
  }

  KRYS_NODISCARD static UnitType TokenUnitType(const Token &token) noexcept
  {
    switch (token.Type())
    {
      case TokenType::Number:     return UnitType::Number;
      case TokenType::Percentage: return UnitType::Percentage;
      case TokenType::Dimension:  return ParseUnitType(token.Unit());
      default:                    return UnitType::Unknown;
    }
  }

  dom::ExceptionOr<Ref<CSSNumericValue>> CSSNumericValue::Parse(dom::Document &document,
                                                                CSSOMString &&cssText) noexcept
  {
    // https://drafts.css-houdini.org/css-typed-om/#dom-cssnumericvalue-parse
    InputStream inputStream {krys::move(cssText)};
    Tokenizer tokenizer(inputStream);
    std::ignore = tokenizer.PumpTokenizer();
    auto range = tokenizer.Tokens();
    range.DiscardWhitespace();
    if (range.IsAtEnd())
    {
      return dom::Exception {dom::ExceptionCode::SyntaxError, u8"Failed to parse CSS text"};
    }
    auto componentValueStart = range;
    range.DiscardComponentValue();
    auto componentValueEnd = range;
    range.DiscardWhitespace();
    if (!range.IsAtEnd())
    {
      return dom::Exception {dom::ExceptionCode::SyntaxError, u8"Failed to parse CSS text"};
    }

    auto componentValueRange = componentValueStart.RangeUntil(componentValueEnd);
    // https://drafts.css-houdini.org/css-typed-om/#reify-a-numeric-value
    switch (componentValueRange.Peek().Type())
    {
      case TokenType::Dimension:
      case TokenType::Number:
      case TokenType::Percentage:
      {
        const auto &token = componentValueRange.ConsumeIncludingWhitespace();
        auto unitType = TokenUnitType(token);
        if (token.Type() == TokenType::Dimension && !CSSNumericType::Create(unitType))
        {
          return dom::Exception {dom::ExceptionCode::SyntaxError, u8"Failed to parse CSS text"};
        }
        return ConvertToExceptionOrNumericValue(CSSUnitValue::Create(token.NumericValue(), unitType));
      }
      case TokenType::Function:
      {
        auto functionId = componentValueRange.Peek().FunctionId();
        if (functionId == ValueId::Calc || functionId == ValueId::Min || functionId == ValueId::Max
            || functionId == ValueId::Clamp)
        {
          // FIXME: The spec is unclear on what context to use when parsing in CSSNumericValue so for the
          // time-being, we use `Category::LengthPercentage`, as it is the most permissive.
          // See https://github.com/w3c/csswg-drafts/issues/10753

          auto parserContext = ParserContext {document};
          auto parserState = PropertyParserState {
            .Context = parserContext,
          };
          auto parserOptions = calc::ParserOptions {
            .Category = Category::LengthPercentage,
            .Range = All,
            .AllowedSymbols = {},
            .PropertyOptions = {},
          };
          auto simplificationOptions = calc::SimplificationOptions {
            .Category = Category::LengthPercentage,
            .Range = All,
            .ConversionData = null,
            .SymbolTable = {},
            .AllowZeroValueLengthRemovalFromSum = false,
          };
          auto tree =
            calc::ParseAndSimplify(componentValueRange, parserState, parserOptions, simplificationOptions);
          if (!tree)
          {
            return dom::Exception {dom::ExceptionCode::SyntaxError, u8"Failed to parse CSS text"};
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
    return dom::Exception {dom::ExceptionCode::SyntaxError, u8"Failed to parse CSS text"};
  }
}

#undef CSS_NUMERIC_RETURN_IF_EXCEPTION
