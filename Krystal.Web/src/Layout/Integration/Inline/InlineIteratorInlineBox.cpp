/*
 * Copyright (C) 2021 Apple Inc. All rights reserved.
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

#include "Krystal.Web/Layout/Integration/Inline/InlineIteratorInlineBox.hpp"
#include "Krystal.Web/Layout/Integration/Inline/LayoutIntegrationLineLayout.hpp"
#include "Krystal.Web/Rendering/RenderBlockFlow.hpp"
#include "Krystal.Web/Rendering/RenderInline.hpp"
#include "Krystal.Web/Rendering/Style/RenderStyle+GettersInlines.hpp"

namespace krys::web::inline_iterator
{
  const RenderBoxModelObject &InlineBox::Renderer() const noexcept
  {
    return Downcast<RenderBoxModelObject>(Box::Renderer());
  }

  RectEdges<bool> InlineBox::ClosedEdges() const noexcept
  {
    // FIXME: Layout knows the answer to this question so we should consult it.
    RectEdges<bool> closedEdges {true};
    if (Style().BoxDecorationBreak() == BoxDecorationBreak::Clone)
    {
      return closedEdges;
    }
    auto writingMode = Style().GetWritingMode();
    bool isFirst = !NextInlineBoxLineLeftward();
    bool isLast = !NextInlineBoxLineRightward();
    closedEdges.SetStart(isFirst, writingMode);
    closedEdges.SetEnd(isLast, writingMode);
    return closedEdges;
  }

  InlineBoxIterator InlineBox::NextInlineBoxLineRightward() const noexcept
  {
    return InlineBoxIterator(*this).TraverseInlineBoxLineRightward();
  }

  InlineBoxIterator InlineBox::NextInlineBoxLineLeftward() const noexcept
  {
    return InlineBoxIterator(*this).TraverseInlineBoxLineLeftward();
  }

  InlineBoxIterator InlineBox::Iterator() const noexcept
  {
    return InlineBoxIterator {*this};
  }

  LeafBoxIterator InlineBox::FirstLeafBox() const noexcept
  {
    return LeafBoxIterator {_path.FirstLeafBoxForInlineBox()};
  }

  LeafBoxIterator InlineBox::LastLeafBox() const noexcept
  {
    return LeafBoxIterator {_path.LastLeafBoxForInlineBox()};
  }

  LeafBoxIterator InlineBox::EndLeafBox() const noexcept
  {
    if (auto last = LastLeafBox())
    {
      return last->NextLineRightwardOnLine();
    }
    return {};
  }

  bool InlineBox::IsSplit() const noexcept
  {
    return NextInlineBoxLineRightward() || NextInlineBoxLineLeftward();
  }

  Pair<BoxIterator, BoxIterator> InlineBox::Descendants() const noexcept
  {
    BoxIterator begin(*this);
    begin.TraverseLineRightwardOnLine();

    BoxIterator end(*this);
    end.TraverseLineRightwardOnLineSkippingChildren();

    return {begin, end};
  }

  InlineBoxIterator &InlineBoxIterator::TraverseInlineBoxLineRightward() noexcept
  {
    _box._path.TraverseNextInlineBox();
    return *this;
  }

  InlineBoxIterator &InlineBoxIterator::TraverseInlineBoxLineLeftward() noexcept
  {
    _box._path.TraversePreviousInlineBox();
    return *this;
  }

  InlineBoxIterator LineLeftmostInlineBoxFor(const RenderInline &renderInline) noexcept
  {
    if (auto *lineLayout = layout_integration::LineLayout::Containing(renderInline))
    {
      return lineLayout->FirstInlineBoxFor(renderInline);
    }
    return {};
  }

  InlineBoxIterator FirstRootInlineBoxFor(const RenderBlockFlow &block) noexcept
  {
    if (auto *lineLayout = block.InlineLayout())
    {
      return lineLayout->FirstRootInlineBox();
    }
    return {};
  }

  InlineBoxIterator InlineBoxFor(const layout_integration::InlineContent &content,
                                 const inline_display::Box &box) noexcept
  {
    return InlineBoxFor(content, content.IndexForBox(box));
  }

  InlineBoxIterator InlineBoxFor(const layout_integration::InlineContent &content, size_t boxIndex) noexcept
  {
    krys_debug_assert(content.DisplayContent().BoxList[boxIndex].IsInlineBox());
    return InlineBoxIterator {InlineContentBoxPath {content, boxIndex}};
  }
}
