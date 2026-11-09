/*
 * Copyright (C) 2017 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/Rendering/Updating/RenderTreeUpdater.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"

namespace krys::boo
{
  enum class PseudoElementType : uint8;
  class RenderElement;
  class RenderQuote;

  /// @brief Updates the `::before` and `::after` pseudo-elements of the elements being updated, their content
  /// renderers, and the quotes and counters.
  // FIXME: Writing suggestions.
  class RenderTreeUpdater::GeneratedContent
  {
    KRYS_NON_COPY_MOVABLE(GeneratedContent);

  private:
    RenderTreeUpdater &_updater;
    WeakPtr<RenderQuote> _previousUpdatedQuote;

  public:
    /// @brief Adds the renderers of the `content` of @p style to @p pseudoRenderer.
    static void CreateContentRenderers(RenderTreeBuilder &builder, RenderElement &pseudoRenderer,
                                       const RenderStyle &style) noexcept;

    explicit GeneratedContent(RenderTreeUpdater &updater) noexcept;

    void UpdateBeforeOrAfterPseudoElement(dom::Element &current, const style::ElementUpdate &elementUpdate,
                                          PseudoElementType pseudoElementType) noexcept;
    void UpdateRemainingQuotes() noexcept;
    void UpdateBackdropRenderer(RenderElement &renderer,
                                style::DifferenceResult minimalStyleDifference) noexcept;
    void UpdateCounters() noexcept;

    static void RemoveBeforePseudoElement(dom::Element &element, RenderTreeBuilder &builder) noexcept;
    static void RemoveAfterPseudoElement(dom::Element &element, RenderTreeBuilder &builder) noexcept;

  private:
    void UpdateQuotesUpTo(RenderQuote *lastQuote) noexcept;

    KRYS_NODISCARD bool NeedsPseudoElement(const RenderStyle *style) noexcept;
  };
}
