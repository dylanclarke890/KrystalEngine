/*
 * Copyright (C) 2023 Igalia S.L. All rights reserved.
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

#include "Krystal.Booey/DOM/ContentVisibility/ContentRelevancy.hpp"
#include "Krystal.Booey/Page/IntersectionObserver/IntersectionObserver.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Types/EnumSet.hpp"
#include "Krystal.Core/Types/HashMap.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::dom
{
  class Document;
  class Element;

  enum class DidUpdateAnyContentRelevancy : bool
  {
    No,
    Yes
  };

  enum class IsSkippedContent : bool
  {
    No,
    Yes
  };

  /// @see https://drafts.csswg.org/css-contain/#proximity-to-the-viewport
  enum class ViewportProximity : bool
  {
    Far,
    Near
  };

  enum class HadInitialVisibleContentVisibilityDetermination : bool
  {
    No,
    Yes
  };

  /// @brief Keeps the relevancy to the user of a document's elements with `content-visibility: auto`: their
  /// proximity to the viewport, from an intersection observer the document makes on first use, and whether
  /// they're focused, selected or hold an element in the top layer.
  /// @see https://drafts.csswg.org/css-contain/#relevant-to-the-user
  class ContentVisibilityDocumentState
  {
    KRYS_NON_COPY_MOVABLE(ContentVisibilityDocumentState);

  private:
    RefPtr<IntersectionObserver> _observer;
    // An element's entry goes when it's unobserved, which its renderer's destruction does.
    HashMap<const Element *, ViewportProximity> _elementViewportProximities;

  public:
    ContentVisibilityDocumentState() noexcept = default;

    /// @brief Starts observing @p element, which has `content-visibility: auto`, with its document's
    /// observer.
    static void Observe(Element &element) noexcept;
    /// @brief Stops observing @p element, and forgets its relevancy.
    static void Unobserve(Element &element) noexcept;

    /// @brief Makes the `content-visibility: auto` ancestor whose contents hide @p scrollAnchor relevant, so
    /// it can be scrolled to.
    void UpdateContentRelevancyForScrollIfNeeded(const Element &scrollAnchor) noexcept;

    KRYS_NODISCARD bool HasObservationTargets() const noexcept
    {
      return _observer && _observer->HasObservationTargets();
    }

    /// @brief Checks the reasons in @p relevancyToCheck for each observed element, updating those whose
    /// relevancy changed.
    DidUpdateAnyContentRelevancy
      UpdateRelevancyOfContentVisibilityElements(EnumSet<ContentRelevancy> relevancyToCheck) const noexcept;
    /// @brief Finds whether the observed elements that haven't had their first observation are on screen.
    /// @return Whether any of them became relevant.
    HadInitialVisibleContentVisibilityDetermination DetermineInitialVisibleContentVisibility() const noexcept;

    void UpdateViewportProximity(const Element &element, ViewportProximity viewportProximity) noexcept;

    /// @brief Has the timelines of the style-originated animations in @p element's subtree pick up their
    /// timing again, when the subtree stops being skipped.
    static void UpdateAnimations(const Element &element, IsSkippedContent wasSkipped,
                                 IsSkippedContent becomesSkipped) noexcept;

  private:
    bool CheckRelevancyOfContentVisibilityElement(Element &target,
                                                  EnumSet<ContentRelevancy> relevancyToCheck) const noexcept;

    void RemoveViewportProximity(const Element &element) noexcept;

    KRYS_NODISCARD IntersectionObserver *GetIntersectionObserver(Document &document) noexcept;
  };
}
