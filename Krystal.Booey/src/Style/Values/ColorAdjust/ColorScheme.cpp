/*
 * Copyright (C) 2024 Samuel Weinig <sam@webkit.org>
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
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "Krystal.Booey/Style/Values/ColorAdjust/ColorScheme.hpp"
#include "Krystal.Booey/CSS/Values/ColorSchemeValue.hpp"
#include "Krystal.Booey/CSS/Values/ToLengthConversionData.hpp"
#include "Krystal.Booey/CSS/Values/ValueId.hpp"
#include "Krystal.Booey/Style/Builder/BuilderChecking.hpp"
#include "Krystal.Booey/Style/Builder/BuilderState.hpp"
#include "Krystal.Core/Text/ASCII.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"

namespace krys::boo::style
{
  EnumSet<boo::ColorScheme> ColorScheme::GetColorScheme() const noexcept
  {
    EnumSet<boo::ColorScheme> result;
    for (const auto &scheme : Schemes)
    {
      if (text::IsASCIICaselessEqual(scheme.Value.View(), css::CSSOMStringView {u8"light"}))
      {
        result.Add(boo::ColorScheme::Light);
      }
      else if (text::IsASCIICaselessEqual(scheme.Value.View(), css::CSSOMStringView {u8"dark"}))
      {
        result.Add(boo::ColorScheme::Dark);
      }
    }
    return result;
  }

  // MARK: - Conversion

  Ref<css::Value> CSSValueCreation<ColorScheme>::operator()(css::ValuePool &, const RenderStyle &style,
                                                            const ColorScheme &value) noexcept
  {
    return css::ColorSchemeValue::Create(ConvertToCSS(value, style));
  }

  ColorScheme CSSValueConversion<ColorScheme>::operator()(BuilderState &state,
                                                          const css::Value &value) noexcept
  {
    auto *colorSchemeValue = RequiredDowncast<css::ColorSchemeValue>(state, value);
    if (!colorSchemeValue)
    {
      return css::keywords::Normal {};
    }
    return ConvertToStyle(colorSchemeValue->GetColorScheme(), state);
  }

  // MARK: - Serialisation

  void Serialiser<ColorScheme>::operator()(StringBuilder &builder, const css::SerialisationContext &context,
                                           const RenderStyle &style, const ColorScheme &value) noexcept
  {
    if (value.IsNormal())
    {
      Serialise(builder, context, style, css::keywords::Normal {});
      return;
    }

    Serialise(builder, context, style, value.Schemes);
    if (value.Only)
    {
      builder.Append(u8" ");
      Serialise(builder, context, style, *value.Only);
    }
  }
}
