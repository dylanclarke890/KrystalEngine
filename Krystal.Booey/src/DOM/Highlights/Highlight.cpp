/*
 * Copyright (C) 2019-2023 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/DOM/Highlights/Highlight.hpp"
#include "Krystal.Booey/DOM/Ranges/SimpleRange.hpp"
#include "Krystal.Booey/Rendering/RenderObject.hpp"
#include <algorithm>

namespace krys::boo::dom
{
  void Highlight::RepaintRange(const AbstractRange &range) noexcept
  {
    auto sortedRange = MakeSimpleRange(range);
    if (std::is_gt(TreeOrder<TreeType::ComposedTree>(sortedRange.Start, sortedRange.End)))
    {
      std::swap(sortedRange.Start, sortedRange.End);
    }
    for (auto &node : IntersectingNodes(sortedRange))
    {
      if (auto *renderer = node.Renderer())
      {
        renderer->Repaint();
      }
    }
  }

  Ref<Highlight> Highlight::Create(List<Ref<AbstractRange>> &&initialRanges) noexcept
  {
    return AdoptRef(*new Highlight(krys::move(initialRanges)));
  }

  Highlight::Highlight(List<Ref<AbstractRange>> &&initialRanges) noexcept
  {
    _highlightRanges.reserve(initialRanges.size());
    for (auto &range : initialRanges)
    {
      RepaintRange(*range);
      _highlightRanges.push_back(HighlightRange::Create(krys::move(range)));
    }
  }

  bool Highlight::Has(const AbstractRange &range) const noexcept
  {
    return std::ranges::any_of(_highlightRanges, [&](const Ref<HighlightRange> &current)
                               { return &current->Range() == &range; });
  }

  bool Highlight::Delete(const AbstractRange &range) noexcept
  {
    auto found = std::ranges::find_if(_highlightRanges, [&](const Ref<HighlightRange> &current)
                                      { return &current->Range() == &range; });
    if (found == _highlightRanges.end())
    {
      return false;
    }
    RepaintRange(range);
    _highlightRanges.erase(found);
    return true;
  }

  void Highlight::Clear() noexcept
  {
    for (auto &highlightRange : _highlightRanges)
    {
      RepaintRange(highlightRange->Range());
    }
    _highlightRanges.clear();
  }

  bool Highlight::Add(AbstractRange &range) noexcept
  {
    auto found = std::ranges::find_if(_highlightRanges, [&](const Ref<HighlightRange> &current)
                                      { return &current->Range() == &range; });
    if (found == _highlightRanges.end())
    {
      RepaintRange(range);
      _highlightRanges.push_back(HighlightRange::Create(ShareRef(range)));
      return true;
    }
    // Move to last since SetLike is an ordered set.
    auto highlightRange = krys::move(*found);
    _highlightRanges.erase(found);
    _highlightRanges.push_back(krys::move(highlightRange));
    return false;
  }

  void Highlight::Repaint() noexcept
  {
    for (auto &highlightRange : _highlightRanges)
    {
      RepaintRange(highlightRange->Range());
    }
  }

  void Highlight::SetPriority(int32 priority) noexcept
  {
    if (_priority == priority)
    {
      return;
    }
    _priority = priority;
    Repaint();
  }
}
