/*
 * Copyright (C) 2024-2025 Samuel Weinig <sam@webkit.org>
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

#include "Krystal.Booey/Style/Values/Color/ContrastColor.hpp"
#include "Krystal.Booey/CSS/Values/Color/ContrastColorResolver.hpp"
#include "Krystal.Booey/DOM/Nodes/Document.hpp"
#include "Krystal.Booey/Style/Values/Color/ColorResolutionState.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"

namespace krys::boo::style
{
  // MARK: - Conversion

  Color ToStyleColor(const css::ContrastColor &unresolved, ColorResolutionState &state) noexcept
  {
    ColorResolutionStateNester nester {state};

    auto color = ToStyleColor(unresolved.Color, state);
    if (!color.IsResolvedColor())
    {
      return Color {ContrastColor {krys::move(color)}};
    }

    return css::Resolve(css::ContrastColorResolver {color.GetResolvedColor()});
  }

  // MARK: - Resolve

  boo::Color ResolveColor(const ContrastColor &contrastColor, const boo::Color &currentColor) noexcept
  {
    return css::Resolve(css::ContrastColorResolver {contrastColor.Color.ResolveColor(currentColor)});
  }

  // MARK: - Current Color

  bool ContainsCurrentColor(const ContrastColor &contrastColor) noexcept
  {
    return style::ContainsCurrentColor(contrastColor.Color);
  }

  // MARK: - Serialisation

  void SerialisationForCSSTokenization(StringBuilder &builder, const css::SerialisationContext &context,
                                       const ContrastColor &contrastColor) noexcept
  {
    builder.Append(u8"contrast-color(");
    SerialisationForCSSTokenization(builder, context, contrastColor.Color);
    builder.Append(u8')');
  }

  css::CSSOMString SerialisationForCSSTokenization(const css::SerialisationContext &context,
                                                   const ContrastColor &contrastColor) noexcept
  {
    StringBuilder builder;
    SerialisationForCSSTokenization(builder, context, contrastColor);
    return krys::move(builder).ToString();
  }
}
