/*
 * Copyright (C) 2014 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/Rendering/Selection/RenderSelectionGeometry.hpp"
#include "Krystal.Booey/Rendering/RenderBlock.hpp"
#include "Krystal.Booey/Rendering/RenderText.hpp"

namespace krys::boo
{
  RenderSelectionGeometryBase::RenderSelectionGeometryBase(RenderObject &renderer) noexcept
      : _renderer(renderer), _repaintContainer(renderer.ContainerForRepaint().Renderer),
        _state(renderer.SelectionState())
  {
  }

  void RenderSelectionGeometryBase::RepaintRectangle(const LayoutRect &repaintRect) noexcept
  {
    _renderer.RepaintUsingContainer(_repaintContainer, LayoutRect(EnclosingIntRect(repaintRect)));
  }

  RenderSelectionGeometry::RenderSelectionGeometry(RenderObject &renderer, bool clipToVisibleContent) noexcept
      : RenderSelectionGeometryBase(renderer)
  {
    if (renderer.CanUpdateSelectionOnRootLineBoxes())
    {
      if (auto *textRenderer = DynamicDowncast<RenderText>(renderer))
      {
        _rect = textRenderer->CollectSelectionGeometriesForLineBoxes(_repaintContainer, clipToVisibleContent,
                                                                     _collectedSelectionQuads);
      }
      else
      {
        _rect = renderer.SelectionRectForRepaint(_repaintContainer, clipToVisibleContent);
      }
    }
  }

  void RenderSelectionGeometry::Repaint() noexcept
  {
    RepaintRectangle(_rect);
  }

  RenderBlockSelectionGeometry::RenderBlockSelectionGeometry(RenderBlock &renderer) noexcept
      : RenderSelectionGeometryBase(renderer),
        _rects(renderer.CanUpdateSelectionOnRootLineBoxes()
                 ? renderer.SelectionGapRectsForRepaint(_repaintContainer)
                 : GapRects())
  {
  }

  void RenderBlockSelectionGeometry::Repaint() noexcept
  {
    RepaintRectangle(_rects);
  }
}
