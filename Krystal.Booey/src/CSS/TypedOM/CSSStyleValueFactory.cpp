/*
 * Copyright (C) 2021 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/CSS/TypedOM/CSSStyleValueFactory.hpp"
#include "Krystal.Booey/CSS/Calc/Value.hpp"
#include "Krystal.Booey/CSS/Parser/InputStream.hpp"
#include "Krystal.Booey/CSS/Parser/KeywordLookup.hpp"
#include "Krystal.Booey/CSS/Parser/Parser.hpp"
#include "Krystal.Booey/CSS/Parser/ParserContext.hpp"
#include "Krystal.Booey/CSS/Parser/Tokenizer.hpp"
#include "Krystal.Booey/CSS/Properties/MutableStyleProperties.hpp"
#include "Krystal.Booey/CSS/Properties/Property.hpp"
#include "Krystal.Booey/CSS/Properties/StylePropertiesInlines.hpp"
#include "Krystal.Booey/CSS/TypedOM/CSSImageValue.hpp"
#include "Krystal.Booey/CSS/TypedOM/CSSKeywordValue.hpp"
#include "Krystal.Booey/CSS/TypedOM/CSSUnitValue.hpp"
#include "Krystal.Booey/CSS/TypedOM/CSSUnparsedValue.hpp"
#include "Krystal.Booey/CSS/TypedOM/Transform/CSSTransformValue.hpp"
#include "Krystal.Booey/CSS/Values/BoxShadowPropertyValue.hpp"
#include "Krystal.Booey/CSS/Values/CustomPropertyValue.hpp"
#include "Krystal.Booey/CSS/Values/EasingFunctionValue.hpp"
#include "Krystal.Booey/CSS/Values/FilterValue.hpp"
#include "Krystal.Booey/CSS/Values/ImageValue.hpp"
#include "Krystal.Booey/CSS/Values/PendingSubstitutionValue.hpp"
#include "Krystal.Booey/CSS/Values/PrimitiveValue.hpp"
#include "Krystal.Booey/CSS/Values/SerialisationContext.hpp"
#include "Krystal.Booey/CSS/Values/TextShadowPropertyValue.hpp"
#include "Krystal.Booey/CSS/Values/TransformListValue.hpp"
#include "Krystal.Booey/CSS/Values/ValueList.hpp"
#include "Krystal.Booey/CSS/Values/VariableData.hpp"
#include "Krystal.Booey/CSS/Values/VariableReferenceValue.hpp"
#include "Krystal.Core/Text/ASCII.hpp"

namespace krys::boo::css
{
  namespace
  {
    KRYS_NODISCARD Ref<CSSStyleValue> ReifyKeyword(ValueId keyword) noexcept
    {
      return CSSKeywordValue::RectifyKeywordish(CSSOMString(ToString(keyword)));
    }

    /// @brief Whether a list value of @p property may be reified as its only item. The parser represents
    /// some values that aren't lists as lists, which the Typed OM doesn't treat as lists.
    KRYS_NODISCARD bool MayConvertValueListToSingleValue(Maybe<PropertyId> property) noexcept
    {
      if (!property)
      {
        return true;
      }
      return *property != PropertyId::GridRowStart && *property != PropertyId::GridRowEnd
             && *property != PropertyId::GridColumnStart && *property != PropertyId::GridColumnEnd;
    }

    /// @brief Reifies @p value as a `CSSUnitValue`, if it's a number, percentage or dimension.
    /// @see https://drafts.css-houdini.org/css-typed-om/#reify-a-numeric-value
    KRYS_NODISCARD RefPtr<CSSStyleValue> ReifyNumericValue(const PrimitiveValue &value) noexcept
    {
      auto type = value.PrimitiveType();
      switch (type)
      {
        case UnitType::Number:
        case UnitType::Integer:
        {
          return CSSUnitValue::Create(value.ValueNoConversionDataRequired<double>(), UnitType::Number);
        }
        case UnitType::Percentage:
        {
          return CSSUnitValue::Create(value.ValueNoConversionDataRequired<double>(), UnitType::Percentage);
        }
        default:
        {
          break;
        }
      }
      // A dimension: a unit the Typed OM can name.
      auto unit = PrimitiveValue::UnitTypeString(type);
      if (unit.empty() || ParseUnitType(unit) != type)
      {
        return nullptr;
      }
      return CSSUnitValue::Create(value.ValueNoConversionDataRequired<double>(), type);
    }

    /// @brief @p value, the value of a list or none property, as `none` if it's that, or a generic style
    /// value.
    template <typename T>
    KRYS_NODISCARD Ref<CSSStyleValue> ReifyListOrNone(const T &list, const Value &value) noexcept
    {
      if (list.IsNone())
      {
        return ReifyKeyword(ValueId::None);
      }
      return CSSStyleValue::Create(ShareRefPtr(const_cast<Value *>(&value)));
    }

    KRYS_NODISCARD dom::ExceptionOr<RefPtr<Value>> ExtractValue(dom::Document &document, PropertyId property,
                                                                CSSOMStringView cssText) noexcept
    {
      auto declaration = MutableStyleProperties::Create();
      if (Parser::ParseValue(*declaration, property, cssText, IsImportant(true), ParserContext(document))
          == Parser::ParseResult::Error)
      {
        return dom::Exception {dom::ExceptionCode::TypeError};
      }
      return declaration->GetPropertyCSSValue(property);
    }

    KRYS_NODISCARD dom::ExceptionOr<RefPtr<CSSStyleValue>>
      ExtractShorthandValues(dom::Document &document, PropertyId property, CSSOMStringView cssText) noexcept
    {
      auto declaration = MutableStyleProperties::Create();
      if (Parser::ParseValue(*declaration, property, cssText, IsImportant(true), ParserContext(document))
          == Parser::ParseResult::Error)
      {
        return dom::Exception {dom::ExceptionCode::TypeError};
      }
      return CSSStyleValueFactory::ConstructStyleValueForShorthandSerialisation(
        document, declaration->GetPropertyValue(property));
    }

    KRYS_NODISCARD Ref<CSSUnparsedValue> ExtractCustomValues(CSSOMStringView cssText) noexcept
    {
      InputStream input {CSSOMString(cssText)};
      Tokenizer tokenizer(input);
      std::ignore = tokenizer.PumpTokenizer();
      return CSSUnparsedValue::Create(tokenizer.Tokens());
    }
  }

  RefPtr<CSSStyleValue> CSSStyleValueFactory::ConstructStyleValueForShorthandSerialisation(
    dom::Document &document, const CSSOMString &serialisation) noexcept
  {
    if (serialisation.empty())
    {
      return nullptr;
    }

    InputStream input {CSSOMString(serialisation)};
    Tokenizer tokenizer(input);
    std::ignore = tokenizer.PumpTokenizer();
    if (serialisation.contains(u8"var("))
    {
      return CSSUnparsedValue::Create(tokenizer.Tokens());
    }
    return CSSStyleValue::Create(VariableReferenceValue::Create(tokenizer.Tokens(), ParserContext(document)));
  }

  dom::ExceptionOr<List<Ref<CSSStyleValue>>>
    CSSStyleValueFactory::ParseStyleValue(dom::Document &document, CSSOMStringView property,
                                          CSSOMStringView cssText, bool parseMultiple) noexcept
  {
    // https://drafts.css-houdini.org/css-typed-om/#parse-a-cssstylevalue
    if (cssText.empty())
    {
      return dom::Exception {dom::ExceptionCode::TypeError};
    }

    // A custom property's value is its tokens, with the variable references among them.
    if (IsCustomPropertyName(property))
    {
      return List<Ref<CSSStyleValue>> {ExtractCustomValues(cssText)};
    }

    // 1. If property is not a custom property name string, set property to property ASCII lowercased.
    // 2. If property is not a valid CSS property, throw a TypeError.
    auto propertyId = PropertyIdFromName(text::ToASCIILower(property));
    if (propertyId == PropertyId::Invalid)
    {
      return dom::Exception {dom::ExceptionCode::TypeError};
    }

    // A shorthand's value is its serialisation.
    if (IsShorthand(propertyId))
    {
      auto result = ExtractShorthandValues(document, propertyId, cssText);
      if (result.HasException())
      {
        return result.ReleaseException();
      }
      auto value = result.ReleaseValue();
      if (!value)
      {
        return List<Ref<CSSStyleValue>> {};
      }
      Ref<CSSStyleValue> nonNullValue = krys::move(value);
      return List<Ref<CSSStyleValue>> {krys::move(nonNullValue)};
    }

    // 3. Attempt to parse cssText according to property's grammar. If this fails, throw a TypeError.
    //    Otherwise, let whatToReturn be the result.
    auto result = ExtractValue(document, propertyId, cssText);
    if (result.HasException())
    {
      return result.ReleaseException();
    }

    // 4. Subdivide into iterations whatToReturn, and set whatToReturn to the result.
    // https://drafts.css-houdini.org/css-typed-om/#subdivide-into-iterations
    List<Ref<Value>> values;
    if (auto value = result.ReleaseValue())
    {
      if (Property::IsListValuedProperty(propertyId))
      {
        if (auto *list = DynamicDowncast<ValueContainingList>(*value))
        {
          for (const auto &item : *list)
          {
            values.push_back(ShareRef(const_cast<Value &>(item)));
          }
        }
      }
      if (values.empty())
      {
        values.push_back(Ref<Value>(krys::move(value)));
      }
    }

    // 5. For each value in whatToReturn, reify it as a CSSStyleValue.
    // 6. If parseMultiple is false, return whatToReturn[0]. Otherwise, return whatToReturn.
    List<Ref<CSSStyleValue>> results;
    for (auto &value : values)
    {
      auto reified = ReifyValue(document, *value, propertyId);
      if (reified.HasException())
      {
        return reified.ReleaseException();
      }
      results.push_back(reified.ReleaseValue());

      if (!parseMultiple)
      {
        break;
      }
    }

    return results;
  }

  dom::ExceptionOr<Ref<CSSStyleValue>> CSSStyleValueFactory::ReifyValue(dom::Document &document,
                                                                        const Value &value,
                                                                        Maybe<PropertyId> property) noexcept
  {
    // https://drafts.css-houdini.org/css-typed-om/#reify-as-a-cssstylevalue
    if (const auto *primitiveValue = DynamicDowncast<PrimitiveValue>(value))
    {
      // https://drafts.css-houdini.org/css-typed-om/#reify-a-math-expression
      if (const auto *calcValue = primitiveValue->CSSCalcValue())
      {
        auto result = CSSNumericValue::ReifyMathExpression(calcValue->GetTree());
        if (result.HasException())
        {
          return result.ReleaseException();
        }
        return Ref<CSSStyleValue>(result.ReleaseValue());
      }
      // https://drafts.css-houdini.org/css-typed-om/#reify-ident
      // The keyword is the identifier's serialisation, so keywords are lowercase.
      if (primitiveValue->IsValueId() || primitiveValue->IsCustomIdent())
      {
        return Ref<CSSStyleValue>(
          CSSKeywordValue::RectifyKeywordish(primitiveValue->CssText(DefaultSerialisationContext())));
      }
      if (auto numeric = ReifyNumericValue(*primitiveValue))
      {
        Ref<CSSStyleValue> nonNullNumeric = krys::move(numeric);
        return nonNullNumeric;
      }
    }
    else if (const auto *imageValue = DynamicDowncast<ImageValue>(value))
    {
      return Ref<CSSStyleValue>(
        CSSImageValue::Create(ShareRef(const_cast<ImageValue &>(*imageValue)), document));
    }
    else if (const auto *referenceValue = DynamicDowncast<VariableReferenceValue>(value))
    {
      // https://drafts.css-houdini.org/css-typed-om/#reify-a-list-of-component-values
      return Ref<CSSStyleValue>(CSSUnparsedValue::Create(referenceValue->Data().TokenRange()));
    }
    else if (const auto *substitutionValue = DynamicDowncast<PendingSubstitutionValue>(value))
    {
      return Ref<CSSStyleValue>(
        CSSUnparsedValue::Create(substitutionValue->ShorthandValue().Data().TokenRange()));
    }
    else if (const auto *customPropertyValue = DynamicDowncast<CustomPropertyValue>(value))
    {
      return krys::SwitchOn(
        customPropertyValue->Value(), [&](const Ref<VariableReferenceValue> &reference)
        { return ReifyValue(document, *reference, property); }, [&](const Ref<VariableData> &data)
        { return ReifyValue(document, *VariableReferenceValue::Create(Ref<VariableData>(data)), property); },
        [&](GlobalKeyword keyword) -> dom::ExceptionOr<Ref<CSSStyleValue>>
        { return ReifyKeyword(ToValueId(keyword)); });
    }
    else if (const auto *transformList = DynamicDowncast<TransformListValue>(value))
    {
      auto transformValue = CSSTransformValue::Create(*transformList, document);
      if (transformValue.HasException())
      {
        return transformValue.ReleaseException();
      }
      return Ref<CSSStyleValue>(transformValue.ReleaseValue());
    }
    else if (const auto *filterValue = DynamicDowncast<FilterValue>(value))
    {
      return ReifyListOrNone(filterValue->GetFilter(), value);
    }
    else if (const auto *boxShadowValue = DynamicDowncast<BoxShadowPropertyValue>(value))
    {
      return ReifyListOrNone(boxShadowValue->Shadow(), value);
    }
    else if (const auto *textShadowValue = DynamicDowncast<TextShadowPropertyValue>(value))
    {
      return ReifyListOrNone(textShadowValue->Shadow(), value);
    }
    else if (const auto *easingValue = DynamicDowncast<EasingFunctionValue>(value))
    {
      // A keyword easing function is that keyword.
      return easingValue->GetEasingFunction().SwitchOn(
        [&]<ValueId keyword>(KeywordValueConstant<keyword>) { return ReifyKeyword(keyword); },
        [&](const auto &) { return CSSStyleValue::Create(ShareRefPtr(const_cast<Value *>(&value))); });
    }
    else if (const auto *valueList = DynamicDowncast<ValueList>(value))
    {
      // A list of one item is that item, and a list-valued property's list is its first item.
      if (!valueList->Length())
      {
        return dom::Exception {dom::ExceptionCode::TypeError};
      }
      if ((valueList->Length() == 1 && MayConvertValueListToSingleValue(property))
          || (property && Property::IsListValuedProperty(*property)))
      {
        return ReifyValue(document, (*valueList)[0], property);
      }
    }

    return CSSStyleValue::Create(ShareRefPtr(const_cast<Value *>(&value)));
  }

  dom::ExceptionOr<List<Ref<CSSStyleValue>>>
    CSSStyleValueFactory::ListFromStyleValuesOrStrings(dom::Document &document, CSSOMStringView property,
                                                       List<CSSStyleValueOrString> values) noexcept
  {
    List<Ref<CSSStyleValue>> styleValues;
    for (auto &value : values)
    {
      if (auto *styleValue = std::get_if<RefPtr<CSSStyleValue>>(&value))
      {
        krys_assert(*styleValue);
        styleValues.push_back(Ref<CSSStyleValue>(krys::move(*styleValue)));
        continue;
      }
      auto result = ParseStyleValue(document, property, std::get<CSSOMString>(value), true);
      if (result.HasException())
      {
        return result.ReleaseException();
      }
      for (auto &parsed : result.ReleaseValue())
      {
        styleValues.push_back(krys::move(parsed));
      }
    }
    return styleValues;
  }
}
