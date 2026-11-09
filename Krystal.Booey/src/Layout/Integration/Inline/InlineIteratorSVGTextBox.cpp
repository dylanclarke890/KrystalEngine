/*
 * Copyright (C) 2024 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/Layout/Integration/Inline/InlineIteratorSVGTextBox.hpp"
#include "Krystal.Booey/Layout/Integration/Inline/LayoutIntegrationLineLayout.hpp"
#include "Krystal.Booey/Rendering/SVG/RenderSVGText.hpp"
#include "Krystal.Booey/Rendering/SVG/SVGInlineTextBox.hpp"
#include "Krystal.Booey/Rendering/SVG/SVGRootInlineBox.hpp"
#include "Krystal.Booey/Rendering/SVG/SVGTextBoxPainter.hpp"
#include "Krystal.Booey/Rendering/SVG/SVGTextFragment.hpp"

namespace krys::boo::inline_iterator
{
  SVGTextBox::SVGTextBox(PathVariant &&path) noexcept : TextBox(krys::move(path))
  {
  }

  FloatRect SVGTextBox::CalculateBoundariesIncludingSVGTransform() const noexcept
  {
    FloatRect textRect;

    float scalingFactor = Renderer().ScalingFactor();
    krys_debug_assert(scalingFactor);

    float baseline = Renderer().ScaledFont().MetricsOfPrimaryFont().Ascent() / scalingFactor;
    for (auto &fragment : TextFragments())
    {
      auto fragmentRect = FloatRect {fragment.X, fragment.Y - baseline, fragment.Width, fragment.Height};

      AffineTransform fragmentTransform;
      fragment.BuildFragmentTransform(fragmentTransform);
      if (!fragmentTransform.IsIdentity())
      {
        fragmentRect = fragmentTransform.MapRect(fragmentRect);
      }

      textRect.Unite(fragmentRect);
    }
    return textRect;
  }

  LayoutRect SVGTextBox::LocalSelectionRect(uint32 start, uint32 end) const noexcept
  {
    auto [clampedStart, clampedEnd] = SelectableRange().Clamp(start, end);

    if (clampedStart >= clampedEnd)
    {
      return LayoutRect();
    }

    AffineTransform fragmentTransform;
    FloatRect selectionRect;
    uint32 fragmentStartPosition = 0;
    uint32 fragmentEndPosition = 0;

    for (auto &fragment : TextFragments())
    {
      fragmentStartPosition = clampedStart;
      fragmentEndPosition = clampedEnd;
      if (!MapStartEndPositionsIntoFragmentCoordinates(Start(), fragment, fragmentStartPosition,
                                                       fragmentEndPosition))
      {
        continue;
      }

      FloatRect fragmentRect = SelectionRectForTextFragment(Renderer(), Direction(), fragment,
                                                            fragmentStartPosition, fragmentEndPosition);
      fragment.BuildFragmentTransform(fragmentTransform);
      if (!fragmentTransform.IsIdentity())
      {
        fragmentRect = fragmentTransform.MapRect(fragmentRect);
      }

      selectionRect.Unite(fragmentRect);
    }

    return EnclosingIntRect(selectionRect);
  }

  const List<SVGTextFragment> &SVGTextBox::TextFragments() const noexcept
  {
    return SwitchOn(_pathVariant, [&](const auto &path) -> const List<SVGTextFragment> &
                    { return path.SvgTextFragments(); });
  }

  const SVGInlineTextBox *SVGTextBox::GetLegacyInlineBox() const noexcept
  {
    return Downcast<SVGInlineTextBox>(TextBox::GetLegacyInlineBox());
  }

  SVGTextBoxIterator::SVGTextBoxIterator(Box::PathVariant &&path) noexcept : TextBoxIterator(krys::move(path))
  {
  }

  SVGTextBoxIterator::SVGTextBoxIterator(const Box &box) noexcept : TextBoxIterator(box)
  {
  }

  SVGTextBoxIterator FirstSVGTextBoxFor(const RenderSVGInlineText &text) noexcept
  {
    if (auto *lineLayout = layout_integration::LineLayout::Containing(text))
    {
      auto box = lineLayout->TextBoxesFor(text);
      if (!box)
      {
        return {};
      }
      return SVGTextBoxIterator {*box};
    }

    return SVGTextBoxIterator {BoxLegacyPath {text.FirstLegacyTextBox()}};
  }

  BoxRange<SVGTextBoxIterator> SVGTextBoxesFor(const RenderSVGInlineText &text) noexcept
  {
    return BoxRange<SVGTextBoxIterator> {FirstSVGTextBoxFor(text)};
  }

  SVGTextBoxIterator SVGTextBoxFor(const SVGInlineTextBox *box) noexcept
  {
    return SVGTextBoxIterator {BoxLegacyPath {box}};
  }

  SVGTextBoxIterator SVGTextBoxFor(const layout_integration::InlineContent &inlineContent,
                                   size_t boxIndex) noexcept
  {
    auto &box = inlineContent.DisplayContent().BoxList[boxIndex];
    if (!box.IsText() || !box.LayoutBox().RendererForIntegration()->IsRenderSVGInlineText())
    {
      return {};
    }
    return SVGTextBoxIterator {BoxModernPath {inlineContent, boxIndex}};
  }

  SVGTextBox::Key MakeKey(const SVGTextBox &textBox) noexcept
  {
    return {&textBox.Renderer(), textBox.Start()};
  }

  BoxRange<BoxIterator> BoxesFor(const RenderSVGText &svgText) noexcept
  {
    if (auto *lineLayout = svgText.InlineLayout())
    {
      return BoxRange<BoxIterator> {BoxIterator {*lineLayout->FirstRootInlineBox()}};
    }

    return BoxRange<BoxIterator> {BoxIterator {BoxLegacyPath {svgText.LegacyRootBox()}}};
  }
}
