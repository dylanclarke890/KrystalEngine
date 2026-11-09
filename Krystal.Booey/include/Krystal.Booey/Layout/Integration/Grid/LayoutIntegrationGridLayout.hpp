/*
 * Copyright (C) 2025 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/Layout/LayoutState.hpp"
#include "Krystal.Booey/Rendering/Grid/RenderGrid.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Types/CheckedPtr.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"

namespace krys::boo::layout
{
  class ElementBox;
}

namespace krys::boo::layout_integration
{
  /// @brief Lays out a grid container's content with the grid formatting context and copies the result back
  /// to the render tree.
  class GridLayout
  {
    KRYS_NON_COPY_MOVABLE(GridLayout);

  private:
    CheckedPtr<layout::ElementBox> _gridBox;
    WeakPtr<layout::LayoutState> _layoutState;

  public:
    explicit GridLayout(RenderGrid &renderGrid) noexcept;
    ~GridLayout() noexcept;

    /// @brief Whether the grid formatting context supports the grid container's content.
    KRYS_NODISCARD static bool CanUseFor(const RenderGrid &renderGrid) noexcept;

    void UpdateFormattingContextGeometries() noexcept;

    void Layout() noexcept;

  private:
    void UpdateGridItemRenderers() noexcept;
    void UpdateFormattingContextRootRenderer() noexcept;

    KRYS_NODISCARD const layout::ElementBox &GridBox() const noexcept
    {
      return *_gridBox;
    }
    KRYS_NODISCARD layout::ElementBox &GridBox() noexcept
    {
      return *_gridBox;
    }

    KRYS_NODISCARD const RenderGrid &GridBoxRenderer() const noexcept;
    KRYS_NODISCARD RenderGrid &GridBoxRenderer() noexcept;

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
