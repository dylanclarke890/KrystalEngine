/*
 * Copyright (C) 2019-2021 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/Layout/Integration/Inline/InlineIteratorTextBox.hpp"
#include "Krystal.Booey/Layout/Integration/Inline/LayoutIntegrationLineLayout.hpp"
#include "Krystal.Booey/Rendering/RenderCombineText.hpp"
#include "Krystal.Booey/Rendering/RenderText.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Rendering/SVG/RenderSVGInlineText.hpp"

namespace krys::boo::inline_iterator
{
  TextBoxIterator TextBox::NextTextBox() const noexcept
  {
    return TextBoxIterator(*this).TraverseNextTextBox();
  }

  const FontCascade &TextBox::GetFontCascade() const noexcept
  {
    if (auto *renderer = DynamicDowncast<RenderCombineText>(Renderer()); renderer && renderer->IsCombined())
    {
      return renderer->TextCombineFont();
    }
    return Style().GetFontCascade();
  }

  const RenderText &TextBox::Renderer() const noexcept
  {
    return Downcast<RenderText>(Box::Renderer());
  }

  TextBoxIterator &TextBoxIterator::TraverseNextTextBox() noexcept
  {
    SwitchOn(_box._pathVariant, [](auto &path) { path.TraverseNextTextBox(); });
    return *this;
  }

  TextBoxIterator LineLeftmostTextBoxFor(const RenderText &text) noexcept
  {
    if (auto *lineLayout = layout_integration::LineLayout::Containing(text))
    {
      return lineLayout->TextBoxesFor(text);
    }
    if (auto *svgText = DynamicDowncast<RenderSVGInlineText>(text))
    {
      return TextBoxIterator {BoxLegacyPath {svgText->FirstLegacyTextBox()}};
    }

    // During teardown we may hit this codepath _after_ the display content is destroyed (e.g. calling repaint
    // on RenderText).
    return TextBoxIterator {BoxLegacyPath {nullptr}};
  }

  TextBoxIterator TextBoxFor(const LegacyInlineTextBox *legacyInlineTextBox) noexcept
  {
    return TextBoxIterator {BoxLegacyPath {legacyInlineTextBox}};
  }

  TextBoxIterator TextBoxFor(const layout_integration::InlineContent &content,
                             const inline_display::Box &box) noexcept
  {
    return TextBoxFor(content, content.IndexForBox(box));
  }

  TextBoxIterator TextBoxFor(const layout_integration::InlineContent &content, size_t boxIndex) noexcept
  {
    krys_debug_assert(content.DisplayContent().BoxList[boxIndex].IsTextOrSoftLineBreak());
    return TextBoxIterator {BoxModernPath {content, boxIndex}};
  }

  BoxRange<TextBoxIterator> TextBoxesFor(const RenderText &text) noexcept
  {
    return BoxRange<TextBoxIterator> {LineLeftmostTextBoxFor(text)};
  }
}
