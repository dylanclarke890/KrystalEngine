#include "Krystal.Booey/Style/Values/TextDecoration/TextDecorationLine.hpp"

#include "Krystal.Booey/CSS/PrimitiveValueMappings.hpp"
#include "Krystal.Booey/CSS/Values/PrimitiveValue.hpp"
#include "Krystal.Booey/CSS/Values/ValueList.hpp"
#include "Krystal.Booey/CSS/Values/ValuePool.hpp"
#include "Krystal.Booey/Graphics/Animation/AnimationUtilities.hpp"
#include "Krystal.Booey/Style/BuilderChecking.hpp"
#include "Krystal.Booey/Style/BuilderState.hpp"
#include "Krystal.Booey/Style/Values/Types.hpp"
#include "Krystal.Core/Assert.hpp"

namespace krys::boo::style
{
  uint8 TextDecorationLine::AddOrReplaceIfNotNone(const TextDecorationLine &value) noexcept
  {
    value.SwitchOn([&](css::keywords::None) {}, [&](css::keywords::SpellingError) { SetSpellingError(); },
                   [&](css::keywords::GrammarError) { SetGrammarError(); },
                   [&](const EnumSet<TextDecorationLine::Flag> &newValue) { SetFlags(newValue); });
    return _packed;
  }

  // MARK: - Conversion

  TextDecorationLine CSSValueConversion<TextDecorationLine>::operator()(BuilderState &state,
                                                                        const css::Value &value) noexcept
  {
    auto invalidValue = [&state]() -> TextDecorationLine
    {
      state.SetCurrentPropertyInvalidAtComputedValueTime();
      return css::keywords::None {};
    };

    if (auto *primitiveValue = DynamicDowncast<css::PrimitiveValue>(value))
    {
      if (primitiveValue->IsValueId())
      {
        switch (primitiveValue->ValueId())
        {
          case css::ValueId::None:          return css::keywords::None {};
          case css::ValueId::SpellingError: return css::keywords::SpellingError {};
          case css::ValueId::GrammarError:  return css::keywords::GrammarError {};
          default:                          break;
        }
      }
      return invalidValue();
    }

    if (auto *valueList = DynamicDowncast<css::ValueList>(value))
    {
      EnumSet<TextDecorationLine::Flag> flags;

      for (const auto &item : *valueList)
      {
        auto *primitiveValue = RequiredDowncast<css::PrimitiveValue>(state, item);
        if (!primitiveValue)
        {
          return invalidValue();
        }

        switch (primitiveValue->ValueId())
        {
          case css::ValueId::Underline:   flags.Add(TextDecorationLine::Flag::Underline); break;
          case css::ValueId::Overline:    flags.Add(TextDecorationLine::Flag::Overline); break;
          case css::ValueId::LineThrough: flags.Add(TextDecorationLine::Flag::LineThrough); break;
          case css::ValueId::Blink:       flags.Add(TextDecorationLine::Flag::Blink); break;
          default:                        return invalidValue();
        }
      }

      if (flags.IsEmpty())
      {
        return invalidValue();
      }

      return flags;
    }

    return invalidValue();
  }

  Ref<css::Value> CSSValueCreation<EnumSet<TextDecorationLine::Flag>>::operator()(
    css::ValuePool &, const RenderStyle &, const EnumSet<TextDecorationLine::Flag> &value) noexcept
  {
    krys_debug_assert(!value.IsEmpty());

    css::ValueListBuilder list;
    if (value.Contains(TextDecorationLine::Flag::Underline))
    {
      list.push_back(css::PrimitiveValue::Create(css::ValueId::Underline));
    }
    if (value.Contains(TextDecorationLine::Flag::Overline))
    {
      list.push_back(css::PrimitiveValue::Create(css::ValueId::Overline));
    }
    if (value.Contains(TextDecorationLine::Flag::LineThrough))
    {
      list.push_back(css::PrimitiveValue::Create(css::ValueId::LineThrough));
    }
    if (value.Contains(TextDecorationLine::Flag::Blink))
    {
      list.push_back(css::PrimitiveValue::Create(css::ValueId::Blink));
    }
    return css::ValueList::CreateSpaceSeparated(krys::move(list));
  }

  // MARK: - Serialization

  void Serialiser<EnumSet<TextDecorationLine::Flag>>::operator()(
    css::CSSOMString &builder, const css::SerialisationContext &, const RenderStyle &,
    const EnumSet<TextDecorationLine::Flag> &value) noexcept
  {
    krys_debug_assert(!value.IsEmpty());

    bool needsSpace = false;
    auto appendOption = [&](TextDecorationLine::Flag option, css::ValueId valueID)
    {
      if (value.Contains(option))
      {
        if (needsSpace)
        {
          builder.push_back(u8' ');
        }
        builder.append(css::ToLower(valueID));
        needsSpace = true;
      }
    };
    appendOption(TextDecorationLine::Flag::Underline, css::ValueId::Underline);
    appendOption(TextDecorationLine::Flag::Overline, css::ValueId::Overline);
    appendOption(TextDecorationLine::Flag::LineThrough, css::ValueId::LineThrough);
    // Blink value is ignored for rendering but not for the computed value.
    appendOption(TextDecorationLine::Flag::Blink, css::ValueId::Blink);
  }
}
