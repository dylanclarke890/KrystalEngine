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

#include "Krystal.Booey/CSS/Highlights/Highlight.hpp"
#include "Krystal.Booey/DOM/Ranges/SimpleRange.hpp"
#include "Krystal.Booey/Rendering/RenderObject.hpp"
#include "Krystal.Core/Move.hpp"
#include <algorithm>

namespace krys::boo
{
  namespace
  {
    KRYS_NODISCARD auto FindHighlightRange(List<Ref<css::HighlightRange>> &ranges,
                                           const dom::AbstractRange &range) noexcept
    {
      return std::ranges::find_if(ranges, [&](const Ref<css::HighlightRange> &current)
                                  { return &current->Range() == &range; });
    }
  }

#pragma region Highlight - https://drafts.csswg.org/css-highlight-api-1/#highlight

  Ref<Highlight> Highlight::Create(List<Ref<dom::AbstractRange>> initialRanges) noexcept
  {
    // 1. Let highlight be the new Highlight object.
    auto highlight = AdoptRef(*new Highlight);

    // 2. Set highlight's priority to 0.
    // 3. Set highlight's type to highlight.
    // (The members' initial values.)

    // 4. For each range of initialRanges, let rangeArg be the result of converting range to an ECMAScript
    // value,
    //    then run the steps for a built-in setlike add function, with highlight as the this value, and
    //    rangeArg as the argument.
    for (auto &range : initialRanges)
    {
      highlight->Add(*range);
    }

    // 5. Return highlight.
    return highlight;
  }

  bool Highlight::Has(const dom::AbstractRange &range) const noexcept
  {
    return std::ranges::any_of(_highlightRanges, [&](const Ref<css::HighlightRange> &current)
                               { return &current->Range() == &range; });
  }

  bool Highlight::Add(dom::AbstractRange &range) noexcept
  {
    // A range that's in the set already stays where it is.
    if (FindHighlightRange(_highlightRanges, range) != _highlightRanges.end())
    {
      return false;
    }

    RepaintRange(range);
    _highlightRanges.push_back(css::HighlightRange::Create(ShareRef(range)));
    return true;
  }

  bool Highlight::Delete(const dom::AbstractRange &range) noexcept
  {
    auto found = FindHighlightRange(_highlightRanges, range);
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
    Repaint();
    _highlightRanges.clear();
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

#pragma endregion

  void Highlight::RepaintRange(const dom::AbstractRange &range) noexcept
  {
    auto sortedRange = dom::CreateSimpleRange(range);
    if (std::is_gt(dom::TreeOrder<dom::TreeType::ComposedTree>(sortedRange.Start, sortedRange.End)))
    {
      std::swap(sortedRange.Start, sortedRange.End);
    }

    for (auto &node : dom::IntersectingNodes(sortedRange))
    {
      if (auto *renderer = node.Renderer())
      {
        renderer->Repaint();
      }
    }
  }

  void Highlight::Repaint() noexcept
  {
    for (auto &highlightRange : _highlightRanges)
    {
      RepaintRange(highlightRange->Range());
    }
  }
}
