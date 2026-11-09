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

#include "Krystal.Booey/Style/Values/Fonts/FontVariantEastAsian.hpp"

#include "Krystal.Booey/CSS/Properties/Consumers/Font.hpp"
#include "Krystal.Booey/Style/Builder/BuilderChecking.hpp"
#include "Krystal.Booey/Style/Builder/BuilderState.hpp"

namespace krys::boo::style
{
  // MARK: - Conversion

  FontVariantEastAsian CSSValueConversion<FontVariantEastAsian>::operator()(BuilderState &state,
                                                                            const css::Value &value) noexcept
  {
    if (const auto *primitiveValue = DynamicDowncast<css::PrimitiveValue>(value))
    {
      switch (primitiveValue->ValueId())
      {
        case css::ValueId::Normal: return css::keywords::Normal {};
        default:                   state.SetCurrentPropertyInvalidAtComputedValueTime(); return css::keywords::Normal {};
      }
    }

    auto list = RequiredListDowncast<css::ValueList, css::PrimitiveValue>(state, value);
    if (!list)
    {
      return css::keywords::Normal {};
    }

    auto variant = FontVariantEastAsianVariant::Normal;
    auto width = FontVariantEastAsianWidth::Normal;
    auto ruby = FontVariantEastAsianRuby::Normal;

    for (const auto &item : *list)
    {
      switch (item.ValueId())
      {
        case css::ValueId::Jis78:             variant = FontVariantEastAsianVariant::Jis78; break;
        case css::ValueId::Jis83:             variant = FontVariantEastAsianVariant::Jis83; break;
        case css::ValueId::Jis90:             variant = FontVariantEastAsianVariant::Jis90; break;
        case css::ValueId::Jis04:             variant = FontVariantEastAsianVariant::Jis04; break;
        case css::ValueId::Simplified:        variant = FontVariantEastAsianVariant::Simplified; break;
        case css::ValueId::Traditional:       variant = FontVariantEastAsianVariant::Traditional; break;
        case css::ValueId::FullWidth:         width = FontVariantEastAsianWidth::Full; break;
        case css::ValueId::ProportionalWidth: width = FontVariantEastAsianWidth::Proportional; break;
        case css::ValueId::Ruby:              ruby = FontVariantEastAsianRuby::Yes; break;
        default:                              state.SetCurrentPropertyInvalidAtComputedValueTime(); return css::keywords::Normal {};
      }
    }

    return FontVariantEastAsian::Platform {variant, width, ruby};
  }
}
