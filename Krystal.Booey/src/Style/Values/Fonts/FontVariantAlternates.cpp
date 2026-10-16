#include "Krystal.Booey/Style/Values/Fonts/FontVariantAlternates.hpp"

#include "Krystal.Booey/CSS/Properties/Consumers/Font.hpp"
#include "Krystal.Booey/CSS/Values/FunctionValue.hpp"
#include "Krystal.Booey/CSS/Values/ValueList.hpp"
#include "Krystal.Booey/Style/BuilderChecking.hpp"
#include "Krystal.Booey/Style/BuilderState.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"

namespace krys::boo::style
{
  // MARK: - Conversion

  FontVariantAlternates
    CSSValueConversion<FontVariantAlternates>::operator()(BuilderState &state,
                                                          const css::Value &value) noexcept
  {
    auto processSingleItemFunction = [&](const css::FunctionValue &function,
                                         Maybe<css::CSSOMString> &parameterToSet) -> bool
    {
      if (function.size() != 1)
      {
        state.SetCurrentPropertyInvalidAtComputedValueTime();
        return false;
      }
      const auto *primitiveArgument = DynamicDowncast<css::PrimitiveValue>(function[0]);
      if (!primitiveArgument || !primitiveArgument->IsCustomIdent())
      {
        state.SetCurrentPropertyInvalidAtComputedValueTime();
        return false;
      }
      parameterToSet = primitiveArgument->StringValue();
      return true;
    };

    auto processListFunction = [&](const css::FunctionValue &function,
                                   List<css::CSSOMString> &parameterToSet) -> bool
    {
      if (!function.size())
      {
        state.SetCurrentPropertyInvalidAtComputedValueTime();
        return false;
      }
      for (const auto &argument : function)
      {
        const auto *primitiveArgument = DynamicDowncast<css::PrimitiveValue>(argument);
        if (!primitiveArgument || !primitiveArgument->IsCustomIdent())
        {
          state.SetCurrentPropertyInvalidAtComputedValueTime();
          return false;
        }
        parameterToSet.push_back(primitiveArgument->StringValue());
      }
      return true;
    };

    if (const auto *primitiveValue = DynamicDowncast<css::PrimitiveValue>(value))
    {
      switch (primitiveValue->ValueId())
      {
        case css::ValueId::Normal:          return css::keywords::Normal {};
        case css::ValueId::HistoricalForms: return css::keywords::HistoricalForms {};
        default:                            state.SetCurrentPropertyInvalidAtComputedValueTime(); return css::keywords::Normal {};
      }
    }

    const auto *list = RequiredDowncast<css::ValueList>(state, value);
    if (!list)
    {
      return css::keywords::Normal {};
    }

    auto result = FontVariantAlternates::Platform::Normal();

    for (const auto &item : *list)
    {
      if (const auto *primitive = DynamicDowncast<css::PrimitiveValue>(item))
      {
        switch (primitive->ValueId())
        {
          case css::ValueId::HistoricalForms: result.ValuesRef().HistoricalForms = true; break;
          default:                            state.SetCurrentPropertyInvalidAtComputedValueTime(); return css::keywords::Normal {};
        }
      }
      else if (const auto *function = DynamicDowncast<css::FunctionValue>(item))
      {
        switch (function->Name())
        {
          case css::ValueId::Swash:
            if (!processSingleItemFunction(*function, result.ValuesRef().Swash))
            {
              return css::keywords::Normal {};
            }
            break;
          case css::ValueId::Stylistic:
            if (!processSingleItemFunction(*function, result.ValuesRef().Stylistic))
            {
              return css::keywords::Normal {};
            }
            break;
          case css::ValueId::Styleset:
            if (!processListFunction(*function, result.ValuesRef().Styleset))
            {
              return css::keywords::Normal {};
            }
            break;
          case css::ValueId::CharacterVariant:
            if (!processListFunction(*function, result.ValuesRef().CharacterVariant))
            {
              return css::keywords::Normal {};
            }
            break;
          case css::ValueId::Ornaments:
            if (!processSingleItemFunction(*function, result.ValuesRef().Ornaments))
            {
              return css::keywords::Normal {};
            }
            break;
          case css::ValueId::Annotation:
            if (!processSingleItemFunction(*function, result.ValuesRef().Annotation))
            {
              return css::keywords::Normal {};
            }
            break;
          default: state.SetCurrentPropertyInvalidAtComputedValueTime(); return css::keywords::Normal {};
        }
      }
      else
      {
        state.SetCurrentPropertyInvalidAtComputedValueTime();
        return css::keywords::Normal {};
      }
    }

    return result;
  }

  Ref<css::Value>
    CSSValueCreation<FontVariantAlternates>::operator()(css::ValuePool &pool, const RenderStyle &style,
                                                        const FontVariantAlternates &alternates) noexcept
  {
    if (alternates.IsNormal())
    {
      return CreateCSSValue(pool, style, css::keywords::Normal {});
    }

    css::ValueListBuilder valueList;

    auto appendKeyword = [&](auto name, const auto &value)
    {
      if (value)
      {
        valueList.push_back(CreateCSSValue(pool, style, name));
      }
    };
    auto appendSingleItemFunction = [&](auto name, const auto &value)
    {
      if (value)
      {
        valueList.push_back(css::FunctionValue::Create(
          name.Value, CreateCSSValue(pool, style, css::CustomIdentifier {css::CSSOMStringAtom {*value}})));
      }
    };
    auto appendListFunction = [&](auto name, const auto &value)
    {
      if (!value.empty())
      {
        css::ValueListBuilder functionArguments;
        for (auto &argument : value)
        {
          functionArguments.push_back(
            CreateCSSValue(pool, style, css::CustomIdentifier {css::CSSOMStringAtom {argument}}));
        }
        valueList.push_back(css::FunctionValue::Create(name.Value, krys::move(functionArguments)));
      }
    };

    appendSingleItemFunction(css::keywords::Stylistic {}, alternates.GetPlatform().GetValues().Stylistic);
    appendKeyword(css::keywords::HistoricalForms {}, alternates.GetPlatform().GetValues().HistoricalForms);
    appendListFunction(css::keywords::Styleset {}, alternates.GetPlatform().GetValues().Styleset);
    appendListFunction(css::keywords::CharacterVariant {},
                       alternates.GetPlatform().GetValues().CharacterVariant);
    appendSingleItemFunction(css::keywords::Swash {}, alternates.GetPlatform().GetValues().Swash);
    appendSingleItemFunction(css::keywords::Ornaments {}, alternates.GetPlatform().GetValues().Ornaments);
    appendSingleItemFunction(css::keywords::Annotation {}, alternates.GetPlatform().GetValues().Annotation);

    if (valueList.size() == 1)
    {
      return valueList[0];
    }
    return css::ValueList::CreateSpaceSeparated(krys::move(valueList));
  }

  void Serialiser<FontVariantAlternates>::operator()(StringBuilder &builder,
                                                     const css::SerialisationContext &context,
                                                     const RenderStyle &style,
                                                     const FontVariantAlternates &alternates) noexcept
  {
    if (alternates.IsNormal())
    {
      Serialise(builder, context, style, css::keywords::Normal {});
      return;
    }

    bool needsSpace = false;

    auto appendKeyword = [&](auto name, const auto &value)
    {
      if (value)
      {
        if (needsSpace)
        {
          builder.Append(u8' ');
        }
        Serialise(builder, context, style, name);
        needsSpace = true;
      }
    };
    auto appendSingleItemFunction = [&](auto name, const auto &value)
    {
      if (value)
      {
        if (needsSpace)
        {
          builder.Append(u8' ');
        }

        Serialise(builder, context, style, name);
        builder.Append(u8'(');
        Serialise(builder, context, style, css::CustomIdentifier {css::CSSOMStringAtom {*value}});
        builder.Append(u8')');

        needsSpace = true;
      }
    };
    auto appendListFunction = [&](auto name, const auto &value)
    {
      if (!value.empty())
      {
        if (needsSpace)
        {
          builder.Append(u8' ');
        }

        Serialise(builder, context, style, name);
        builder.Append(u8'(');
        bool first = true;
        for (const auto &argument : value)
        {
          if (!first)
          {
            builder.Append(u8", ");
          }
          first = false;
          Serialise(builder, context, style, css::CustomIdentifier {css::CSSOMStringAtom {argument}});
        }
        builder.Append(u8')');

        needsSpace = true;
      }
    };

    appendSingleItemFunction(css::keywords::Stylistic {}, alternates.GetPlatform().GetValues().Stylistic);
    appendKeyword(css::keywords::HistoricalForms {}, alternates.GetPlatform().GetValues().HistoricalForms);
    appendListFunction(css::keywords::Styleset {}, alternates.GetPlatform().GetValues().Styleset);
    appendListFunction(css::keywords::CharacterVariant {},
                       alternates.GetPlatform().GetValues().CharacterVariant);
    appendSingleItemFunction(css::keywords::Swash {}, alternates.GetPlatform().GetValues().Swash);
    appendSingleItemFunction(css::keywords::Ornaments {}, alternates.GetPlatform().GetValues().Ornaments);
    appendSingleItemFunction(css::keywords::Annotation {}, alternates.GetPlatform().GetValues().Annotation);
  }
}
