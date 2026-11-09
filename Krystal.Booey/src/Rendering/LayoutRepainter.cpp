/*
 * Copyright (C) 2000 Lars Knoll (knoll@kde.org)
 *           (C) 2000 Antti Koivisto (koivisto@kde.org)
 *           (C) 2000 Dirk Mueller (mueller@kde.org)
 *           (C) 2004 Allan Sandfeld Jensen (kde@carewolf.com)
 * Copyright (C) 2003, 2004, 2005, 2006, 2007, 2008, 2009 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/Rendering/LayoutRepainter.hpp"
#include "Krystal.Booey/Rendering/RenderLayerModelObject.hpp"

namespace krys::boo
{
  LayoutRepainter::LayoutRepainter(RenderElement &renderer, Maybe<CheckForRepaint> checkForRepaintOverride,
                                   Maybe<ShouldAlwaysIssueFullRepaint> shouldAlwaysIssueFullRepaint,
                                   RepaintOutlineBounds repaintOutlineBounds) noexcept
      : _renderer(renderer),
        _checkForRepaint(checkForRepaintOverride ? *checkForRepaintOverride == CheckForRepaint::Yes
                                                 : renderer.CheckForRepaintDuringLayout()),
        _forceFullRepaint(shouldAlwaysIssueFullRepaint
                          && *shouldAlwaysIssueFullRepaint == ShouldAlwaysIssueFullRepaint::Yes),
        _repaintOutlineBounds(repaintOutlineBounds)
  {
    if (!_checkForRepaint)
    {
      return;
    }

    _repaintContainer = _renderer.ContainerForRepaint().Renderer;
    _oldRects = _renderer.RectsForRepaintingAfterLayout(_repaintContainer, _repaintOutlineBounds);
  }

  bool LayoutRepainter::RepaintAfterLayout() noexcept
  {
    if (!_checkForRepaint)
    {
      return false;
    }

    auto requiresFullRepaint =
      _forceFullRepaint || _renderer.SelfNeedsLayout() ? RequiresFullRepaint::Yes : RequiresFullRepaint::No;
    // Outline bounds are not used if we're doing a full repaint.
    auto newRects = _renderer.RectsForRepaintingAfterLayout(
      _repaintContainer,
      (requiresFullRepaint == RequiresFullRepaint::Yes) ? RepaintOutlineBounds::No : _repaintOutlineBounds);
    return _renderer.RepaintAfterLayoutIfNeeded(_repaintContainer, requiresFullRepaint, _oldRects, newRects);
  }
}
