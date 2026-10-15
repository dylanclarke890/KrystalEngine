#include "Krystal.Booey/Style/CustomProperty.hpp"
#include "Krystal.Booey/CSS/Parser/InputStream.hpp"
#include "Krystal.Booey/CSS/Parser/Tokenizer.hpp"
#include "Krystal.Booey/CSS/Values/PrimitiveValue.hpp"
#include "Krystal.Booey/CSS/Values/SerialisationContext.hpp"
#include "Krystal.Booey/CSS/Values/ValueList.hpp"
#include "Krystal.Booey/CSS/Values/ValueListBuilder.hpp"
#include "Krystal.Booey/CSS/Values/ValuePool.hpp"
#include "Krystal.Booey/CSS/Values/VariableReferenceValue.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Style/Seam.hpp"
#include "Krystal.Booey/Style/Values/Primitives/KeywordCSSValueCreation.hpp"
#include "Krystal.Booey/Style/Values/Primitives/KeywordSerialisation.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesCSSValueCreation.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesSerialisation.hpp"
#include "Krystal.Core/Utils/PointerComparison.hpp"

namespace krys::boo::style
{
  bool CustomProperty::operator==(const CustomProperty &other) const noexcept
  {
    if (_name != other._name || _value.index() != other._value.index())
    {
      return false;
    }

    return krys::SwitchOn(
      _value, [&](const GuaranteedInvalid &) { return true; }, [&](const Ref<css::VariableData> &value)
      { return ArePointingToEqualData(value, std::get<Ref<css::VariableData>>(other._value)); },
      [&](const Value &value) { return value == std::get<Value>(other._value); },
      [&](const ValueList &value) { return value == std::get<ValueList>(other._value); });
  }

  Ref<css::Value> CustomProperty::PropertyValue(css::ValuePool &pool, const RenderStyle &style) const noexcept
  {
    auto convertValue = [&](const Value &value)
    {
      return krys::SwitchOn(value, [&](const auto &alternative) -> Ref<css::Value>
                            { return CreateCSSValue(pool, style, alternative); });
    };

    return krys::SwitchOn(
      _value, [&](const GuaranteedInvalid &) -> Ref<css::Value>
      { return css::PrimitiveValue::Create(css::CSSOMString {}); },
      [&](const Ref<css::VariableData> &variableData) -> Ref<css::Value>
      { return css::VariableReferenceValue::Create(Ref<css::VariableData> {variableData}); },
      [&](const Value &value) -> Ref<css::Value> { return convertValue(value); },
      [&](const ValueList &valueList) -> Ref<css::Value>
      {
        css::ValueListBuilder builder;
        for (const auto &value : valueList.Values)
        {
          builder.push_back(convertValue(value));
        }
        switch (valueList.Separator)
        {
          case css::ValueSeparator::SpaceSeparator:
            return css::ValueList::CreateSpaceSeparated(krys::move(builder));
          case css::ValueSeparator::CommaSeparator:
            return css::ValueList::CreateCommaSeparated(krys::move(builder));
          case css::ValueSeparator::SlashSeparator:
            return css::ValueList::CreateSlashSeparated(krys::move(builder));
        }
        krys_unreachable();
      });
  }

  css::CSSOMString CustomProperty::PropertyValueSerialisation(const css::SerialisationContext &context,
                                                              const RenderStyle &style) const noexcept
  {
    css::CSSOMString builder;
    PropertyValueSerialisation(builder, context, style);
    return builder;
  }

  void CustomProperty::PropertyValueSerialisation(css::CSSOMString &builder,
                                                  const css::SerialisationContext &context,
                                                  const RenderStyle &style) const noexcept
  {
    auto serialiseValue = [&](const Value &value)
    {
      krys::SwitchOn(value, [&](const auto &alternative)
                     { style::Serialise(builder, context, style, alternative); });
    };

    krys::SwitchOn(
      _value,
      [&](const GuaranteedInvalid &)
      {
        // "The guaranteed-invalid value serializes as the empty string".
        // https://drafts.csswg.org/css-variables-2/#guaranteed-invalid
      },
      [&](const Ref<css::VariableData> &value) { builder.append(value->Serialise()); },
      [&](const Value &value) { serialiseValue(value); },
      [&](const ValueList &valueList)
      {
        auto separator = css::Value::SeparatorCSSText(valueList.Separator);
        for (size_t i = 0; i < valueList.Values.size(); ++i)
        {
          if (i)
          {
            builder.append(separator);
          }
          serialiseValue(valueList.Values[i]);
        }
      });
  }

  css::CSSOMString
    CustomProperty::PropertyValueSerialisationForTokenization(const css::SerialisationContext &context,
                                                              const RenderStyle &style) const noexcept
  {
    css::CSSOMString builder;
    PropertyValueSerialisationForTokenization(builder, context, style);
    return builder;
  }

  void CustomProperty::PropertyValueSerialisationForTokenization(css::CSSOMString &builder,
                                                                 const css::SerialisationContext &context,
                                                                 const RenderStyle &style) const noexcept
  {
    // `PropertyValueSerialisationForTokenization` differs from `PropertyValueSerialisation` only in how it
    // handles custom `color` values:
    //
    //   `PropertyValueSerialisation`: serializes the used/resolved value of `color` values (needed for
    //   getComputedStyle)
    //   `PropertyValueSerialisationForTokenization`: serializes the computed value of `color` values (needed
    //   for style building)

    auto serialiseValue = [&](const Value &value)
    {
      krys::SwitchOn(
        value, [&](const Color &color) { style::SerialisationForCSSTokenization(builder, context, color); },
        [&](const auto &alternative) { style::Serialise(builder, context, style, alternative); });
    };

    krys::SwitchOn(
      _value,
      [&](const GuaranteedInvalid &)
      {
        // "The guaranteed-invalid value serializes as the empty string".
        // https://drafts.csswg.org/css-variables-2/#guaranteed-invalid
      },
      [&](const Ref<css::VariableData> &value) { builder.append(value->Serialise()); },
      [&](const Value &value) { serialiseValue(value); },
      [&](const ValueList &valueList)
      {
        auto separator = css::Value::SeparatorCSSText(valueList.Separator);
        for (size_t i = 0; i < valueList.Values.size(); ++i)
        {
          if (i)
          {
            builder.append(separator);
          }
          serialiseValue(valueList.Values[i]);
        }
      });
  }

  const List<css::Token> &CustomProperty::Tokens() const noexcept
  {
    return krys::SwitchOn(
      _value,
      [&](const Ref<css::VariableData> &value) -> const List<css::Token> & { return value->Tokens(); },
      [&](const auto &) -> const List<css::Token> &
      {
        if (!_cachedTokens)
        {
          css::InputStream input {PropertyValueSerialisationForTokenization(
            css::DefaultSerialisationContext(), RenderStyle::DefaultStyleSingleton())};
          css::Tokenizer tokenizer {input};
          std::ignore = tokenizer.PumpTokenizer();
          _cachedTokens = css::VariableData::Create(tokenizer.Tokens());
        }
        return _cachedTokens->Tokens();
      });
  }
}
