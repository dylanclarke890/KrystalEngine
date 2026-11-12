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

#include "Krystal.Booey/Rendering/Updating/RenderTreeBuilder.hpp"

namespace krys::boo
{
  class RenderBlockFlow;
  class RenderFragmentedFlow;
  class RenderMultiColumnFlow;

  /// @brief Creates and destroys the multi-column flows of multi-column containers, and moves spanners
  /// (`column-span: all` boxes) out of the flows to sit among the column sets, making the sets around them.
  class RenderTreeBuilder::MultiColumn
  {
    KRYS_NON_COPY_MOVABLE(MultiColumn);

  private:
    RenderTreeBuilder &_builder;

  public:
    explicit MultiColumn(RenderTreeBuilder &builder) noexcept;

    /// @brief Makes or removes @p flow's multi-column flow, as its style asks for columns or not.
    void UpdateAfterDescendants(RenderBlockFlow &flow) noexcept;
    /// @brief Some renderers (column spanners) are moved out of the flow to live among column sets. If
    /// @p beforeChild is such a renderer, resolves it to the placeholder that lives at its original place in
    /// the tree.
    KRYS_NODISCARD RenderObject *ResolveMovedChild(RenderFragmentedFlow &enclosingFragmentedFlow,
                                                   RenderObject *beforeChild) noexcept;
    /// @brief Moves the spanners whose placeholders are in @p container back to their placeholders.
    void RestoreColumnSpannersForContainer(RenderMultiColumnFlow &multiColumnFlow,
                                           const RenderElement &container) noexcept;
    /// @brief Moves out any spanners in @p newDescendant's subtree, just inserted into @p flow, and makes the
    /// column sets it needs.
    void MultiColumnDescendantInserted(RenderMultiColumnFlow &flow, RenderObject &newDescendant) noexcept;
    /// @brief Forgets @p relative, a child of @p flow's container about to be removed: a placeholder, or a
    /// spanner (merging the sets either side of it).
    void MultiColumnRelativeWillBeRemoved(RenderMultiColumnFlow &flow, RenderObject &relative,
                                          CanCollapseAnonymousBlock canCollapseAnonymousBlock) noexcept;
    /// @brief The placeholder of @p beforeChild, a spanner just before a column set; @p beforeChild
    /// otherwise.
    KRYS_NODISCARD static RenderObject *
      AdjustBeforeChildForMultiColumnSpannerIfNeeded(RenderObject &beforeChild) noexcept;

  private:
    void CreateFragmentedFlow(RenderBlockFlow &flow) noexcept;
    void DestroyFragmentedFlow(RenderBlockFlow &flow) noexcept;
    RenderObject *ProcessPossibleSpannerDescendant(RenderMultiColumnFlow &flow, RenderObject *&subtreeRoot,
                                                   RenderObject &descendant) noexcept;
    void HandleSpannerRemoval(RenderMultiColumnFlow &flow, RenderObject &spanner,
                              CanCollapseAnonymousBlock canCollapseAnonymousBlock) noexcept;
  };
}
