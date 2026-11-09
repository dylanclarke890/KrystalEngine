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

#include "Krystal.Booey/Graphics/Geometry/LayoutUnit.hpp"
#include "Krystal.Booey/Layout/LayoutState.hpp"
#include "Krystal.Booey/Rendering/RenderFlexibleBox.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Types/CheckedPtr.hpp"
#include "Krystal.Core/Types/Pair.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"

namespace krys::boo
{
  class RenderBlock;
  class RenderStyle;
}

namespace krys::boo::layout
{
  class ElementBox;
}

namespace krys::boo::layout_integration
{
  /// @brief Lays out a flex container's content with the flex formatting context and copies the result back
  /// to the render tree.
  // FIXME: Painting and hit testing (paint, hitTest).
  class FlexLayout
  {
    KRYS_NON_COPY_MOVABLE(FlexLayout);

  private:
    CheckedPtr<layout::ElementBox> _flexBox;
    WeakPtr<layout::LayoutState> _layoutState;

  public:
    explicit FlexLayout(RenderFlexibleBox &flexBoxRenderer) noexcept;
    ~FlexLayout() noexcept;

    /// @brief Whether the flex formatting context supports the flex container's content.
    KRYS_NODISCARD static bool CanUseFor(const RenderFlexibleBox &flexBox) noexcept;

    void UpdateFormattingContextGeometries() noexcept;
    void UpdateStyle(const RenderBlock &renderer, const RenderStyle &oldStyle) noexcept;
    KRYS_NODISCARD Pair<LayoutUnit, LayoutUnit> ComputeIntrinsicWidthConstraints() noexcept;

    void Layout() noexcept;

    void CollectOverflow() noexcept;
    KRYS_NODISCARD LayoutUnit ContentBoxLogicalHeight() const noexcept;

  private:
    void UpdateRenderers() noexcept;

    KRYS_NODISCARD const layout::ElementBox &FlexBox() const noexcept
    {
      return *_flexBox;
    }
    KRYS_NODISCARD layout::ElementBox &FlexBox() noexcept
    {
      return *_flexBox;
    }

    KRYS_NODISCARD const RenderFlexibleBox &FlexBoxRenderer() const noexcept;
    KRYS_NODISCARD RenderFlexibleBox &FlexBoxRenderer() noexcept;

    KRYS_NODISCARD layout::LayoutState &GetLayoutState() noexcept
    {
      return *_layoutState;
    }
    KRYS_NODISCARD const layout::LayoutState &GetLayoutState() const noexcept
    {
      return *_layoutState;
    }
  };
}
