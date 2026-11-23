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
#include "Krystal.Booey/CSS/Properties/PropertyQueries.hpp"
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
#include "Krystal.Core/Strings/ASCII.hpp"

namespace krys::boo
{
  namespace
  {
    KRYS_NODISCARD Ref<CSSStyleValue> ReifyKeyword(css::ValueId keyword) noexcept
    {
      return CSSKeywordValue::RectifyKeywordish(CSSOMString(ToString(keyword)));
    }

    /// @brief Whether a list value of @p property may be reified as its only item. The parser represents
    /// some values that aren't lists as lists, which the Typed OM doesn't treat as lists.
    KRYS_NODISCARD bool MayConvertValueListToSingleValue(Maybe<css::PropertyId> property) noexcept
    {
      if (!property)
      {
        return true;
      }
      return *property != css::PropertyId::GridRowStart && *property != css::PropertyId::GridRowEnd
             && *property != css::PropertyId::GridColumnStart && *property != css::PropertyId::GridColumnEnd;
    }

    /// @brief Reifies @p value as a `CSSUnitValue`, if it's a number, percentage or dimension.
    /// @see https://drafts.css-houdini.org/css-typed-om/#reify-a-numeric-value
    KRYS_NODISCARD RefPtr<CSSStyleValue> ReifyNumericValue(const css::PrimitiveValue &value) noexcept
    {
      auto type = value.Unit();
      switch (type)
      {
        case css::UnitType::Number:
        case css::UnitType::Integer:
        {
          return CSSUnitValue::Create(value.ResolveWithoutConversionData<double>(), css::UnitType::Number);
        }
        case css::UnitType::Percentage:
        {
          return CSSUnitValue::Create(value.ResolveWithoutConversionData<double>(),
                                      css::UnitType::Percentage);
        }
        default:
        {
          break;
        }
      }
      // A dimension: a unit the Typed OM can name.
      auto unit = css::UnitTypeString(type);
      if (unit.empty() || css::ParseUnitType(unit) != type)
      {
        return nullptr;
      }
      return CSSUnitValue::Create(value.ResolveWithoutConversionData<double>(), type);
    }

    /// @brief @p value, the value of a list or none property, as `none` if it's that, or a generic style
    /// value.
    template <typename T>
    KRYS_NODISCARD Ref<CSSStyleValue> ReifyListOrNone(const T &list, const css::Value &value) noexcept
    {
      if (list.IsNone())
      {
        return ReifyKeyword(css::ValueId::None);
      }
      return CSSStyleValue::Create(ShareRefPtr(const_cast<css::Value *>(&value)));
    }

    KRYS_NODISCARD ExceptionOr<RefPtr<css::Value>> ExtractValue(Document &document, css::PropertyId property,
                                                                CSSOMStringView cssText) noexcept
    {
      auto declaration = css::MutableStyleProperties::Create();
      if (css::Parser::ParseValue(*declaration, property, cssText, css::IsImportant::Yes,
                                  css::ParserContext(document))
          == css::Parser::ParseResult::Error)
      {
        return Exception {ExceptionCode::TypeError};
      }
      return declaration->GetPropertyCSSValue(property);
    }

    KRYS_NODISCARD ExceptionOr<RefPtr<CSSStyleValue>>
      ExtractShorthandValues(Document &document, css::PropertyId property, CSSOMStringView cssText) noexcept
    {
      auto declaration = css::MutableStyleProperties::Create();
      if (css::Parser::ParseValue(*declaration, property, cssText, css::IsImportant::Yes,
                                  css::ParserContext(document))
          == css::Parser::ParseResult::Error)
      {
        return Exception {ExceptionCode::TypeError};
      }
      return CSSStyleValueFactory::ConstructStyleValueForShorthandSerialisation(
        document, declaration->GetPropertyValue(property));
    }

    KRYS_NODISCARD Ref<CSSUnparsedValue> ExtractCustomValues(CSSOMStringView cssText) noexcept
    {
      css::InputStream input {CSSOMString(cssText)};
      css::Tokenizer tokenizer(input);
      std::ignore = tokenizer.PumpTokenizer();
      return CSSUnparsedValue::Create(tokenizer.Tokens());
    }
  }

  RefPtr<CSSStyleValue> CSSStyleValueFactory::ConstructStyleValueForShorthandSerialisation(
    Document &document, const CSSOMString &serialisation) noexcept
  {
    if (serialisation.empty())
    {
      return nullptr;
    }

    css::InputStream input {CSSOMString(serialisation)};
    css::Tokenizer tokenizer(input);
    std::ignore = tokenizer.PumpTokenizer();
    if (serialisation.contains(u8"var("))
    {
      return CSSUnparsedValue::Create(tokenizer.Tokens());
    }
    return CSSStyleValue::Create(
      css::VariableReferenceValue::Create(tokenizer.Tokens(), css::ParserContext(document)));
  }

  ExceptionOr<List<Ref<CSSStyleValue>>> CSSStyleValueFactory::ParseStyleValue(Document &document,
                                                                              CSSOMStringView property,
                                                                              CSSOMStringView cssText,
                                                                              bool parseMultiple) noexcept
  {
    // https://drafts.css-houdini.org/css-typed-om/#parse-a-cssstylevalue
    if (cssText.empty())
    {
      return Exception {ExceptionCode::TypeError};
    }

    // A custom property's value is its tokens, with the variable references among them.
    if (css::IsCustomPropertyName(property))
    {
      return List<Ref<CSSStyleValue>> {ExtractCustomValues(cssText)};
    }

    // 1. If property is not a custom property name string, set property to property ASCII lowercased.
    // 2. If property is not a valid CSS property, throw a TypeError.
    auto propertyId = css::PropertyIdFromName(text::ToASCIILower(property));
    if (propertyId == css::PropertyId::Invalid)
    {
      return Exception {ExceptionCode::TypeError};
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
    List<Ref<css::Value>> values;
    if (auto value = result.ReleaseValue())
    {
      if (css::IsListValuedProperty(propertyId))
      {
        if (auto *list = DynamicDowncast<css::ValueContainingList>(*value))
        {
          for (const auto &item : *list)
          {
            values.push_back(ShareRef(const_cast<css::Value &>(item)));
          }
        }
      }
      if (values.empty())
      {
        values.emplace_back(krys::move(value));
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

  ExceptionOr<Ref<CSSStyleValue>> CSSStyleValueFactory::ReifyValue(Document &document,
                                                                   const css::Value &value,
                                                                   Maybe<css::PropertyId> property) noexcept
  {
    // https://drafts.css-houdini.org/css-typed-om/#reify-as-a-cssstylevalue
    if (const auto *primitiveValue = DynamicDowncast<css::PrimitiveValue>(value))
    {
      // https://drafts.css-houdini.org/css-typed-om/#reify-a-math-expression
      if (const auto *calcValue = primitiveValue->CalcValue())
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
          CSSKeywordValue::RectifyKeywordish(primitiveValue->CssText(css::DefaultSerialisationContext())));
      }
      if (auto numeric = ReifyNumericValue(*primitiveValue))
      {
        Ref<CSSStyleValue> nonNullNumeric = krys::move(numeric);
        return nonNullNumeric;
      }
    }
    else if (const auto *imageValue = DynamicDowncast<css::ImageValue>(value))
    {
      return Ref<CSSStyleValue>(
        CSSImageValue::Create(ShareRef(const_cast<css::ImageValue &>(*imageValue)), document));
    }
    else if (const auto *referenceValue = DynamicDowncast<css::VariableReferenceValue>(value))
    {
      // https://drafts.css-houdini.org/css-typed-om/#reify-a-list-of-component-values
      return Ref<CSSStyleValue>(CSSUnparsedValue::Create(referenceValue->Data().TokenRange()));
    }
    else if (const auto *substitutionValue = DynamicDowncast<css::PendingSubstitutionValue>(value))
    {
      return Ref<CSSStyleValue>(
        CSSUnparsedValue::Create(substitutionValue->ShorthandValue().Data().TokenRange()));
    }
    else if (const auto *customPropertyValue = DynamicDowncast<css::CustomPropertyValue>(value))
    {
      return krys::SwitchOn(
        customPropertyValue->Value(), [&](const Ref<css::VariableReferenceValue> &reference)
        { return ReifyValue(document, *reference, property); },
        [&](const Ref<css::VariableData> &data)
        {
          return ReifyValue(document, *css::VariableReferenceValue::Create(Ref<css::VariableData>(data)),
                            property);
        },
        [&](css::GlobalKeyword keyword) -> ExceptionOr<Ref<CSSStyleValue>>
        { return ReifyKeyword(css::ToValueId(keyword)); });
    }
    else if (const auto *transformList = DynamicDowncast<css::TransformListValue>(value))
    {
      auto transformValue = CSSTransformValue::Create(*transformList, document);
      if (transformValue.HasException())
      {
        return transformValue.ReleaseException();
      }
      return Ref<CSSStyleValue>(transformValue.ReleaseValue());
    }
    else if (const auto *filterValue = DynamicDowncast<css::FilterValue>(value))
    {
      return ReifyListOrNone(filterValue->GetFilter(), value);
    }
    else if (const auto *boxShadowValue = DynamicDowncast<css::BoxShadowPropertyValue>(value))
    {
      return ReifyListOrNone(boxShadowValue->Shadow(), value);
    }
    else if (const auto *textShadowValue = DynamicDowncast<css::TextShadowPropertyValue>(value))
    {
      return ReifyListOrNone(textShadowValue->Shadow(), value);
    }
    else if (const auto *easingValue = DynamicDowncast<css::EasingFunctionValue>(value))
    {
      // A keyword easing function is that keyword.
      return easingValue->GetEasingFunction().SwitchOn(
        [&]<css::ValueId keyword>(css::KeywordValueConstant<keyword>) { return ReifyKeyword(keyword); },
        [&](const auto &) { return CSSStyleValue::Create(ShareRefPtr(const_cast<css::Value *>(&value))); });
    }
    else if (const auto *valueList = DynamicDowncast<css::ValueList>(value))
    {
      // A list of one item is that item, and a list-valued property's list is its first item.
      if (!valueList->size())
      {
        return Exception {ExceptionCode::TypeError};
      }
      if ((valueList->size() == 1 && MayConvertValueListToSingleValue(property))
          || (property && css::IsListValuedProperty(*property)))
      {
        return ReifyValue(document, (*valueList)[0], property);
      }
    }

    return CSSStyleValue::Create(ShareRefPtr(const_cast<css::Value *>(&value)));
  }

  ExceptionOr<List<Ref<CSSStyleValue>>>
    CSSStyleValueFactory::ListFromStyleValuesOrStrings(Document &document, CSSOMStringView property,
                                                       List<CSSStyleValueOrString> values) noexcept
  {
    List<Ref<CSSStyleValue>> styleValues;
    for (auto &value : values)
    {
      if (auto *styleValue = std::get_if<RefPtr<CSSStyleValue>>(&value))
      {
        krys_assert(*styleValue);
        styleValues.emplace_back(krys::move(*styleValue));
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
