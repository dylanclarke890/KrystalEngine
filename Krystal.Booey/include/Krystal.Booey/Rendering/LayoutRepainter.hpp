/*
 * Copyright (C) 2000 Lars Knoll (knoll@kde.org)
 *           (C) 2000 Antti Koivisto (koivisto@kde.org)
 *           (C) 2000 Dirk Mueller (mueller@kde.org)
 *           (C) 2004 Allan Sandfeld Jensen (kde@carewolf.com)
 * Copyright (C) 2003-2025 Apple Inc. All rights reserved.
 * Copyright (C) 2009 Google Inc. All rights reserved.
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

#include "Krystal.Booey/Rendering/RenderElement.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Types/Maybe.hpp"

namespace krys::boo
{
  class RenderLayerModelObject;

  /// @brief Repaints a renderer after its layout: its rects before layout are taken when it's made, and
  /// `RepaintAfterLayout` repaints what changed between them and the rects after.
  class LayoutRepainter
  {
    KRYS_NON_COPY_MOVABLE(LayoutRepainter);

  public:
    enum class CheckForRepaint : uint8
    {
      No,
      Yes
    };
    enum class ShouldAlwaysIssueFullRepaint : uint8
    {
      No,
      Yes
    };

  private:
    RenderElement &_renderer;
    const RenderLayerModelObject *_repaintContainer {nullptr};
    // We store these values as LayoutRects, but the final invalidations will be pixel snapped
    RenderObject::RepaintRects _oldRects;
    bool _checkForRepaint {true};
    bool _forceFullRepaint {false};
    RepaintOutlineBounds _repaintOutlineBounds;

  public:
    explicit LayoutRepainter(RenderElement &renderer, Maybe<CheckForRepaint> checkForRepaintOverride = null,
                             Maybe<ShouldAlwaysIssueFullRepaint> shouldAlwaysIssueFullRepaint = null,
                             RepaintOutlineBounds repaintOutlineBounds = RepaintOutlineBounds::Yes) noexcept;

    /// @brief Repaints what layout changed.
    /// @return Whether it repainted.
    bool RepaintAfterLayout() noexcept;
  };
}
