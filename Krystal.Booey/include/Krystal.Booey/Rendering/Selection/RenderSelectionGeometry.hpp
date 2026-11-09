/*
 * Copyright (C) 2000 Lars Knoll (knoll@kde.org)
 *           (C) 2000 Antti Koivisto (koivisto@kde.org)
 *           (C) 2000 Dirk Mueller (mueller@kde.org)
 *           (C) 2004 Allan Sandfeld Jensen (kde@carewolf.com)
 * Copyright (C) 2003, 2004, 2005, 2006, 2007, 2008, 2009 Apple Inc. All rights reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 */

#pragma once

#include "Krystal.Booey/Graphics/Geometry/FloatQuad.hpp"
#include "Krystal.Booey/Graphics/Geometry/LayoutRect.hpp"
#include "Krystal.Booey/Rendering/Painting/GapRects.hpp"
#include "Krystal.Booey/Rendering/RenderObject.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Types/List.hpp"

namespace krys::boo
{
  class RenderBlock;
  class RenderLayerModelObject;

  /// @brief A renderer's selection state and repaint container, captured when the selection changes.
  class RenderSelectionGeometryBase
  {
    KRYS_NON_COPYABLE(RenderSelectionGeometryBase);

  protected:
    RenderObject &_renderer;
    const RenderLayerModelObject *_repaintContainer;

  private:
    RenderObject::HighlightState _state;

  public:
    explicit RenderSelectionGeometryBase(RenderObject &renderer) noexcept;

    KRYS_NODISCARD const RenderLayerModelObject *RepaintContainer() const noexcept
    {
      return _repaintContainer;
    }
    KRYS_NODISCARD RenderObject::HighlightState State() const noexcept
    {
      return _state;
    }

  protected:
    void RepaintRectangle(const LayoutRect &repaintRect) noexcept;
  };

  // This struct is used when the selection changes to cache the old and new state of the selection for each
  // RenderObject.
  class RenderSelectionGeometry : public RenderSelectionGeometryBase
  {
  private:
    List<FloatQuad> _collectedSelectionQuads; // relative to repaint container
    LayoutRect _rect;                         // relative to repaint container

  public:
    RenderSelectionGeometry(RenderObject &renderer, bool clipToVisibleContent) noexcept;

    void Repaint() noexcept;
    KRYS_NODISCARD const List<FloatQuad> &CollectedSelectionQuads() const noexcept
    {
      return _collectedSelectionQuads;
    }
    KRYS_NODISCARD LayoutRect Rect() const noexcept
    {
      return _rect;
    }
  };

  // This struct is used when the selection changes to cache the old and new state of the selection for each
  // RenderBlock.
  class RenderBlockSelectionGeometry : public RenderSelectionGeometryBase
  {
  private:
    GapRects _rects; // relative to repaint container

  public:
    explicit RenderBlockSelectionGeometry(RenderBlock &renderer) noexcept;

    void Repaint() noexcept;
    KRYS_NODISCARD GapRects Rects() const noexcept
    {
      return _rects;
    }
  };
}
