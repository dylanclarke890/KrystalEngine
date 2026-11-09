/*
 * Copyright (C) 2025 Apple Inc. All rights reserved.
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
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "Krystal.Booey/Style/Values/TextDecoration/TextDecorationLine.hpp"

#include "Krystal.Booey/CSS/Values/PrimitiveValue.hpp"
#include "Krystal.Booey/CSS/Values/PrimitiveValueMappings.hpp"
#include "Krystal.Booey/CSS/Values/ValueList.hpp"
#include "Krystal.Booey/CSS/Values/ValuePool.hpp"
#include "Krystal.Booey/Graphics/Animation/AnimationUtilities.hpp"
#include "Krystal.Booey/Style/Builder/BuilderChecking.hpp"
#include "Krystal.Booey/Style/Builder/BuilderState.hpp"
#include "Krystal.Booey/Style/Values/Types.hpp"
#include "Krystal.Core/Assert.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"

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
    StringBuilder &builder, const css::SerialisationContext &, const RenderStyle &,
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
          builder.Append(u8' ');
        }
        builder.Append(css::ToLower(valueID));
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
