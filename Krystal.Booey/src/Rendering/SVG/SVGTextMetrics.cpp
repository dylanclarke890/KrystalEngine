/*
 * Copyright (C) Research In Motion Limited 2010-2012. All rights reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "Krystal.Booey/Rendering/SVG/SVGTextMetrics.hpp"
#include "Krystal.Booey/Graphics/TextRuns/TextRun.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Rendering/SVG/RenderSVGInlineText.hpp"
#include "Krystal.Booey/Text/UnicodeBidi.hpp"

namespace krys::boo
{
  SVGTextMetrics::SVGTextMetrics(const RenderSVGInlineText &textRenderer, const TextRun &run) noexcept
  {
    float scalingFactor = textRenderer.ScalingFactor();
    krys_debug_assert(scalingFactor);

    const FontCascade &scaledFont = textRenderer.ScaledFont();

    // Calculate width/height using the scaled font, divide this result by the scalingFactor afterwards.
    _width = scaledFont.Width(run) / scalingFactor;
    _height = scaledFont.MetricsOfPrimaryFont().Height() / scalingFactor;

    _length = static_cast<uint32>(run.Length());
  }

  TextRun SVGTextMetrics::ConstructTextRun(const RenderSVGInlineText &text, uint32 position,
                                           uint32 length) noexcept
  {
    const RenderStyle &style = text.Style();

    TextRun run(utf8_stringview(text.Text()).substr(position, length),
                0, /* xPos, only relevant with allowTabs=true */
                0, /* padding, only relevant for justified text, not relevant for SVG */
                ExpansionBehavior::AllowRightOnly(), style.GetWritingMode().BidiDirection(),
                IsOverride(style.GetUnicodeBidi()) /* directionalOverride */);

    // We handle letter & word spacing ourselves.
    run.DisableSpacing();
    return run;
  }

  SVGTextMetrics SVGTextMetrics::MeasureCharacterRange(const RenderSVGInlineText &text, uint32 position,
                                                       uint32 length) noexcept
  {
    return SVGTextMetrics(text, ConstructTextRun(text, position, length));
  }

  SVGTextMetrics::SVGTextMetrics(const RenderSVGInlineText &text, uint32 length, float width) noexcept
  {
    float scalingFactor = text.ScalingFactor();
    krys_debug_assert(scalingFactor);

    _width = width / scalingFactor;
    _height = text.ScaledFont().MetricsOfPrimaryFont().Height() / scalingFactor;
    _length = length;
  }
}
