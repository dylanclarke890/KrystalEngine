/*
 * Copyright (C) 2022 Apple Inc. All rights reserved.
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

#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Geometry/Point.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Memory/WeakPtr.hpp"

namespace krys::boo
{
  class Document;
  class Element;
  class FrameView;
  class RenderObject;
  class ScrollableArea;

  /// @brief What choosing an anchor does with an element.
  /// @see https://drafts.csswg.org/css-scroll-anchoring/#candidate-examination
  enum class CandidateExaminationResult : uint8
  {
    Exclude,
    Select,
    Descend,
    Skip,
  };

  /// @brief A scroller's scroll anchoring: it keeps an element in its scrollport (the anchor node) where it
  /// is as layout moves it, by scrolling as far as it moved.
  /// @note The anchor is chosen when the scroller scrolls, and after each layout if it has none; the
  /// adjustments are made after layout.
  /// @see https://drafts.csswg.org/css-scroll-anchoring/
  class ScrollAnchoringController
  {
    KRYS_NON_COPY_MOVABLE(ScrollAnchoringController);

  private:
    ScrollableArea &_owningScrollableArea;
    WeakPtr<Element> _anchorElement;
    PointF _lastOffsetForAnchorElement;
    bool _midUpdatingScrollPositionForAnchorElement {false};
    bool _isQueuedForScrollPositionUpdate {false};
    bool _shouldSuppressScrollPositionUpdate {false};

  public:
    explicit ScrollAnchoringController(ScrollableArea &owningScrollableArea) noexcept;

    ~ScrollAnchoringController() noexcept = default;

    /// @brief Forgets the anchor node, and the anchor node of the scroller around this one if its chain
    /// passes through this scroller.
    void InvalidateAnchorElement() noexcept;

    /// @brief Scrolls by as far as the anchor node moved since it was chosen, unless a suppression trigger
    /// happened, which chooses a new anchor node instead.
    /// @see https://drafts.csswg.org/css-scroll-anchoring/#scroll-adjustment
    void AdjustScrollPositionForAnchoring() noexcept;

    /// @brief Chooses the anchor node: a priority candidate, or the first element the candidate examination
    /// selects.
    /// @see https://drafts.csswg.org/css-scroll-anchoring/#anchor-node-selection
    void ChooseAnchorElement(Document &document) noexcept;

    /// @see https://drafts.csswg.org/css-scroll-anchoring/#candidate-examination
    KRYS_NODISCARD CandidateExaminationResult ExamineAnchorCandidate(Element &element) noexcept;

    /// @brief Chooses an anchor node if there's none and the scroller is scrolled, and queues the scroller
    /// for an adjustment after the next layout.
    void UpdateAnchorElement() noexcept;

    /// @brief Notes a suppression trigger: the next adjustment chooses a new anchor node instead.
    /// @see https://drafts.csswg.org/css-scroll-anchoring/#suppression-triggers
    void NotifyChildHadSuppressingStyleChange() noexcept;

    /// @brief Whether @p object renders the anchor node or one of its ancestors within this scroller.
    KRYS_NODISCARD bool IsInScrollAnchoringAncestorChain(const RenderObject &object) const noexcept;

    KRYS_NODISCARD Element *AnchorElement() const noexcept
    {
      return _anchorElement.get();
    }

  private:
    KRYS_NODISCARD Element *FindAnchorElementRecursive(Element *element) noexcept;

    /// @see https://drafts.csswg.org/css-scroll-anchoring/#priority-candidates
    KRYS_NODISCARD bool DidFindPriorityCandidate(Document &document) noexcept;

    KRYS_NODISCARD PointF ComputeOffsetFromOwningScroller(const RenderObject &candidate) const noexcept;

    KRYS_NODISCARD boo::FrameView &FrameView() const noexcept;
  };
}
