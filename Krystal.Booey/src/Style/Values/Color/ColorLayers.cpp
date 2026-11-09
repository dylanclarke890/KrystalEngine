/*
 * Copyright (C) 2024 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/Style/Values/Color/ColorLayers.hpp"
#include "Krystal.Booey/CSS/Values/Color/ColorLayersResolver.hpp"
#include "Krystal.Booey/CSS/Values/PrimitiveValueMappings.hpp"
#include "Krystal.Booey/DOM/Nodes/Document.hpp"
#include "Krystal.Booey/Style/Values/Color/ColorResolutionState.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"
#include <algorithm>

namespace krys::boo::style
{
  namespace
  {
    KRYS_NODISCARD List<boo::Color> ResolveLayers(const ColorLayers &colorLayers,
                                                  const boo::Color &currentColor) noexcept
    {
      // FIXME: This should be made into a lazy transformed range to avoid the unnecessary temporary
      // allocation.
      List<boo::Color> colors;
      colors.reserve(colorLayers.Colors.size());
      for (const auto &color : colorLayers.Colors)
      {
        colors.push_back(color.ResolveColor(currentColor));
      }
      return colors;
    }
  }

  // MARK: - Conversion

  Color ToStyleColor(const css::ColorLayers &unresolved, ColorResolutionState &state) noexcept
  {
    ColorResolutionStateNester nester {state};

    auto colors = css::CommaSeparatedList<Color>::map(unresolved.Colors, [&](const auto &color) -> Color
                                                      { return ToStyleColor(color, state); });

    if (!std::ranges::all_of(colors, [](const auto &color) { return color.IsResolvedColor(); }))
    {
      // If the any of the layer's colors are not resolved, we cannot fully resolve the
      // color yet. Instead we return a style::ColorLayers to be resolved at use time.
      return Color {ColorLayers {.BlendMode = unresolved.BlendMode, .Colors = krys::move(colors)}};
    }

    // FIXME: This should be made into a lazy transformed range to avoid the unnecessary temporary allocation.
    List<boo::Color> resolvedColors;
    resolvedColors.reserve(colors.size());
    for (const auto &color : colors)
    {
      resolvedColors.push_back(color.GetResolvedColor());
    }

    return css::BlendSourceOver(
      css::ColorLayersResolver {.BlendMode = unresolved.BlendMode, .Colors = krys::move(resolvedColors)});
  }

  // MARK: Resolve

  boo::Color ResolveColor(const ColorLayers &colorLayers, const boo::Color &currentColor) noexcept
  {
    return css::BlendSourceOver(css::ColorLayersResolver {
      .BlendMode = colorLayers.BlendMode, .Colors = ResolveLayers(colorLayers, currentColor)});
  }

  // MARK: - Current Color

  bool ContainsCurrentColor(const ColorLayers &colorLayers) noexcept
  {
    return std::ranges::any_of(colorLayers.Colors,
                               [&](const auto &color) { return style::ContainsCurrentColor(color); });
  }

  // MARK: - Serialisation

  void SerialisationForCSSTokenization(StringBuilder &builder, const css::SerialisationContext &context,
                                       const ColorLayers &value) noexcept
  {
    builder.Append(u8"color-layers(");

    if (value.BlendMode != BlendMode::Normal)
    {
      builder.Append(css::ToLower(ToValueId(value.BlendMode)));
      builder.Append(u8", ");
    }

    auto first = true;
    for (const auto &color : value.Colors)
    {
      if (!first)
      {
        builder.Append(u8", ");
      }
      first = false;
      SerialisationForCSSTokenization(builder, context, color);
    }

    builder.Append(u8')');
  }

  css::CSSOMString SerialisationForCSSTokenization(const css::SerialisationContext &context,
                                                   const ColorLayers &colorLayers) noexcept
  {
    StringBuilder builder;
    SerialisationForCSSTokenization(builder, context, colorLayers);
    return krys::move(builder).ToString();
  }
}
