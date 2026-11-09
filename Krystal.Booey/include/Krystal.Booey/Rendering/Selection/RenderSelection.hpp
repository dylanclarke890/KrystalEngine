/*
 * Copyright (C) 2020 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above
 *    copyright notice, this list of conditions and the following
 *    disclaimer.
 * 2. Redistributions in binary form must reproduce the above
 *    copyright notice, this list of conditions and the following
 *    disclaimer in the documentation and/or other materials
 *    provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER “AS IS” AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,
 * OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
 * TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
 * THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#pragma once

#include "Krystal.Booey/Graphics/Geometry/IntRect.hpp"
#include "Krystal.Booey/Rendering/Selection/RenderHighlight.hpp"
#include "Krystal.Core/Attributes.hpp"

namespace krys::boo
{
  class RenderView;

  /// @brief The frame's selection in the render tree: the renderers' selection states, and repainting what a
  /// change adds and removes.
  // FIXME: The selection geometry gatherer (SelectionGeometryGatherer), with service controls.
  class RenderSelection : public RenderHighlight
  {
  public:
    enum class RepaintMode : uint8
    {
      NewXOROld,
      NewMinusOld,
      Nothing
    };

  private:
    enum class ClipToVisibleContent : bool
    {
      No,
      Yes
    };

    const RenderView &_renderView;
    bool _selectionWasCaret {false};

  public:
    explicit RenderSelection(RenderView &view) noexcept;

    void Set(const RenderRange &selection, RepaintMode blockRepaintMode = RepaintMode::NewXOROld) noexcept;
    void Clear() noexcept;
    void Repaint() const noexcept;

    KRYS_NODISCARD IntRect Bounds() const noexcept
    {
      return CollectBounds(ClipToVisibleContent::No);
    }
    KRYS_NODISCARD IntRect BoundsClippedToVisibleContent() const noexcept
    {
      return CollectBounds(ClipToVisibleContent::Yes);
    }

  private:
    KRYS_NODISCARD IntRect CollectBounds(ClipToVisibleContent clipToVisibleContent) const noexcept;
    void Apply(const RenderRange &newSelection, RepaintMode blockRepaintMode) noexcept;
  };
}
