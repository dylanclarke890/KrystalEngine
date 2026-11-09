/*
 * Copyright (C) 2025 Samuel Weinig <sam@webkit.org>
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
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "Krystal.Booey/Page/IntersectionObserver/IntersectionObserverMarginBox.hpp"
#include "Krystal.Booey/Rendering/RepaintRectCalculation.hpp"
#include "Krystal.Core/Enum.hpp"
#include "Krystal.Core/Types/EnumSet.hpp"
#include "Krystal.Core/Types/Maybe.hpp"

namespace krys::boo
{
  /// @brief How a rect is mapped up to a container: which clips and scroll positions apply.
  enum class VisibleRectContextOption : uint8
  {
    UseEdgeInclusiveIntersection,
    ApplyCompositedClips,
    ApplyCompositedContainerScrolls,
    ApplyContainerClip,
    CalculateAccurateRepaintRect
  };
}

KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(krys::boo::VisibleRectContextOption, 5uz);

namespace krys::boo
{
  /// @brief The state of mapping a rect up to a container, through `ComputeVisibleRectsInContainer`.
  struct VisibleRectContext
  {
    using Option = VisibleRectContextOption;

    bool HasPositionFixedDescendant {false};
    bool DirtyRectIsFlipped {false};
    bool DescendantNeedsEnclosingIntRect {false};
    EnumSet<Option> Options {};
    /// @brief An intersection observer's scroll margin, which grows the clips of the scroll containers on the
    /// way.
    Maybe<IntersectionObserverMarginBox> ScrollMargin {};

    KRYS_NODISCARD RepaintRectCalculation GetRepaintRectCalculation() const noexcept
    {
      return Options.Contains(Option::CalculateAccurateRepaintRect) ? RepaintRectCalculation::Accurate
                                                                    : RepaintRectCalculation::Fast;
    }
  };
}
