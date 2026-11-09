/*
 * Copyright (C) 2025 Samuel Weinig <sam@webkit.org>
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
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "Krystal.Booey/Style/Values/Fonts/FontFeatureSettings.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/Font.hpp"
#include "Krystal.Booey/CSS/Values/FontFeatureValue.hpp"
#include "Krystal.Booey/CSS/Values/ValueList.hpp"
#include "Krystal.Booey/Style/Builder/BuilderChecking.hpp"
#include "Krystal.Booey/Style/Builder/BuilderState.hpp"
#include "Krystal.Booey/Style/Values/Fonts/FontOpentypeTag.hpp"
#include "Krystal.Booey/Style/Values/Primitives/KeywordCSSValueCreation.hpp"
#include "Krystal.Booey/Style/Values/Primitives/KeywordSerialisation.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesCSSValueConversion.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesCSSValueCreation.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesSerialisation.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"

namespace krys::boo::style
{
  // MARK: - Conversion

  FontFeatureSettings CSSValueConversion<FontFeatureSettings>::operator()(BuilderState &state,
                                                                          const css::Value &value) noexcept
  {
    if (const auto *primitiveValue = DynamicDowncast<css::PrimitiveValue>(value))
    {
      switch (auto valueID = primitiveValue->ValueId(); valueID)
      {
        case css::ValueId::Normal: return css::keywords::Normal {};
        default:
          if (css::PropertyParserHelpers::IsSystemFontShorthand(valueID))
          {
            return css::keywords::Normal {};
          }
          state.SetCurrentPropertyInvalidAtComputedValueTime();
          return css::keywords::Normal {};
      }
    }

    auto list = RequiredListDowncast<css::ValueList, css::FontFeatureValue>(state, value);
    if (!list)
    {
      return css::keywords::Normal {};
    }

    boo::FontFeatureSettings platformSettings;
    for (const auto &setting : *list)
    {
      platformSettings.insert(
        {setting.Tag(), ConvertCSSValueToStyle<FontFeatureSettings::Value>(state, setting.GetValue()).Value});
    }

    return {krys::move(platformSettings)};
  }

  Ref<css::Value> CSSValueCreation<FontFeatureSettings>::operator()(css::ValuePool &pool,
                                                                    const RenderStyle &style,
                                                                    const FontFeatureSettings &value) noexcept
  {
    if (value.Platform().isEmpty())
    {
      return CreateCSSValue(pool, style, css::keywords::Normal {});
    }

    css::ValueListBuilder list;
    for (const auto &setting : value.Platform())
    {
      list.push_back(css::FontFeatureValue::Create(
        setting.Tag(), CreateCSSValue(pool, style, FontFeatureSettings::Value {setting.Value()})));
    }
    return css::ValueList::CreateCommaSeparated(krys::move(list));
  }

  // MARK: - Serialization

  void Serialiser<FontFeatureSettings>::operator()(StringBuilder &builder,
                                                   const css::SerialisationContext &context,
                                                   const RenderStyle &style,
                                                   const FontFeatureSettings &value) noexcept
  {
    if (value.Platform().isEmpty())
    {
      Serialise(builder, context, style, css::keywords::Normal {});
      return;
    }

    bool first = true;
    for (const auto &setting : value.Platform())
    {
      if (!first)
      {
        builder.Append(u8", ");
      }
      first = false;
      Serialise(builder, context, style, FontOpentypeTag {setting.Tag()});
      if (setting.Value() != 1)
      {
        builder.Append(u8' ');
        Serialise(builder, context, style, FontFeatureSettings::Value {setting.Value()});
      }
    }
  }
}
