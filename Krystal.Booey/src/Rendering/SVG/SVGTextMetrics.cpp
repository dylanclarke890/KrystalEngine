#include "Krystal.Booey/Rendering/SVG/SVGTextMetrics.hpp"
#include "Krystal.Booey/Core/UnicodeBidi.hpp"
#include "Krystal.Booey/Graphics/TextRun.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Rendering/SVG/RenderSVGInlineText.hpp"

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
