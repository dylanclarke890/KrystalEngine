/*
 * Copyright (C) 2019-2020 Apple Inc. All rights reserved.
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

#include "Krystal.Web/Css/Highlights/HighlightRegistry.hpp"
#include "Krystal.Core/Move.hpp"
#include "Krystal.Web/Css/Geometry/DOMRectList.hpp"
#include "Krystal.Web/Dom/Nodes/Document.hpp"
#include "Krystal.Web/Dom/Nodes/ShadowRoot.hpp"
#include "Krystal.Web/Dom/Ranges/Range.hpp"
#include "Krystal.Web/Dom/Ranges/StaticRange.hpp"
#include "Krystal.Web/Dom/Traversal/TreeTraversal.hpp"
#include <algorithm>

namespace krys::web
{
  namespace
  {
    KRYS_NODISCARD DOMStringAtom AnnotationHighlightKey() noexcept
    {
      return u8"annotationHighlightKey";
    }
  }

  HighlightRegistry::HighlightRegistry(Document &document) noexcept : _document(CreateWeakPtr(&document))
  {
  }

  Ref<HighlightRegistry> HighlightRegistry::Create(Document &document) noexcept
  {
    return AdoptRef(*new HighlightRegistry(document));
  }

#pragma region HighlightRegistry - https://drafts.csswg.org/css-highlight-api-1/#highlightregistry

  void HighlightRegistry::Set(const DOMStringAtom &key, Ref<Highlight> value) noexcept
  {
    // Adding a highlight to the registry, or replacing one, repaints.
    // https://drafts.csswg.org/css-highlight-api-1/#repaint
    value->Repaint();
    if (auto found = _map.find(key); found != _map.end())
    {
      found->second->Repaint();
      found->second = krys::move(value);
      return;
    }

    _map.emplace(key, krys::move(value));
    _highlightNames.push_back(key);
  }

  Highlight *HighlightRegistry::Get(const DOMStringAtom &key) const noexcept
  {
    auto found = _map.find(key);
    return found == _map.end() ? nullptr : found->second.get();
  }

  bool HighlightRegistry::Delete(const DOMStringAtom &key) noexcept
  {
    auto found = _map.find(key);
    if (found == _map.end())
    {
      return false;
    }

    // Removing a highlight from the registry repaints.
    found->second->Repaint();
    _map.erase(found);
    _highlightNames.erase(std::ranges::find(_highlightNames, key));
    return true;
  }

  void HighlightRegistry::Clear() noexcept
  {
    // Removing the highlights from the registry repaints; the highlights themselves keep their ranges.
    for (auto &[name, highlight] : _map)
    {
      highlight->Repaint();
    }
    _map.clear();
    _highlightNames.clear();
  }

  List<HighlightHitResult>
    HighlightRegistry::HighlightsFromPoint(float x, float y,
                                           const HighlightsFromPointOptions &options) const noexcept
  {
    RefPtr<Document> document = ShareRefPtr(_document.get());
    if (!document)
    {
      return {};
    }

    // 1. If any of the following are true, return the empty sequence:
    //    - x is negative
    //    - y is negative
    //    - x is greater than the viewport width excluding the size of a rendered scroll bar (if any)
    //    - y is greater than the viewport height excluding the size of a rendered scroll bar (if any)
    //    (NodeFromPoint is null outside the visible content rect.)
    //    - The topmost box in the viewport in paint order that would be a target for hit testing at
    //    coordinates
    //      x,y when applying the transforms that apply to the descendants of the viewport, has an element
    //      associated to it that's in a shadow tree whose shadow root is not contained by
    //      options.shadowRoots.
    auto hitNode = document->NodeFromPoint(LayoutPoint(PointF(x, y)), nullptr, HitTestSource::Script);
    if (!hitNode)
    {
      return {};
    }
    if (auto *shadowRoot = dom::TreeTraversal::ContainingShadowRoot(*hitNode);
        shadowRoot
        && std::ranges::none_of(options.ShadowRoots,
                                [&](const Ref<ShadowRoot> &allowed) { return allowed.get() == shadowRoot; }))
    {
      return {};
    }

    // 2. Otherwise, let results be an empty sequence.
    List<HighlightHitResult> results;

    // 3. For each Highlight highlight in this HighlightRegistry:
    for (auto &name : _highlightNames)
    {
      auto &highlight = _map.find(name)->second;

      // 3a. Let result be a new HighlightHitResult with highlight set to highlight.
      HighlightHitResult result {highlight, {}};

      // 3b. For each AbstractRange abstractRange in highlight:
      for (auto &highlightRange : highlight->HighlightRanges())
      {
        auto &abstractRange = highlightRange->Range();

        // 3b.i. If abstractRange is an invalid StaticRange, then continue.
        if (auto *staticRange = DynamicDowncast<dom::StaticRange>(abstractRange);
            staticRange && !staticRange->IsValid())
        {
          continue;
        }

        // 3b.ii. Let range be a new Range whose start node and end node are set to abstractRange's start node
        //        and end node respectively, and start offset and end offset are set to abstractRange's start
        //        offset and end offset respectively.
        auto range = CreateRef<dom::Range>(
          dom::BoundaryPoint {ShareRef(*abstractRange.StartContainer()), abstractRange.StartOffset()},
          dom::BoundaryPoint {ShareRef(*abstractRange.EndContainer()), abstractRange.EndOffset()});

        // 3b.iii. If the coordinates x,y fall inside at least one of the DOMRects returned by calling
        //         getClientRects() on range, then append abstractRange to result.ranges.
        auto rects = range->GetClientRects();
        for (uint32 i = 0; i < rects->Length(); ++i)
        {
          auto &rect = *rects->Item(i);
          if (x >= rect.Left() && x < rect.Right() && y >= rect.Top() && y < rect.Bottom())
          {
            result.Ranges.push_back(ShareRef(abstractRange));
            break;
          }
        }
      }

      // 3c. If result.ranges is not empty, append result to results.
      if (!result.Ranges.empty())
      {
        results.push_back(krys::move(result));
      }
    }

    // 4. Sort results by descending order of priority of its HighlightHitResults' highlight attributes.
    std::ranges::stable_sort(results, std::ranges::greater {},
                             [](const HighlightHitResult &result) { return result.Highlight->Priority(); });

    // 5. Return results.
    return results;
  }

#pragma endregion

  void HighlightRegistry::AddAnnotationHighlightWithRange(Ref<dom::StaticRange> value) noexcept
  {
    if (auto *highlight = Get(AnnotationHighlightKey()))
    {
      highlight->Add(*value);
    }
    else
    {
      List<Ref<dom::AbstractRange>> ranges;
      ranges.emplace_back(krys::move(value));
      Set(AnnotationHighlightKey(), Highlight::Create(krys::move(ranges)));
    }
  }
}
