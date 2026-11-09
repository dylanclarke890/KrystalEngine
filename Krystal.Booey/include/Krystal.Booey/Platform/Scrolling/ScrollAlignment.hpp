/*
 * Copyright (C) 2003, 2009, 2016 Apple Inc. All rights reserved.
 *
 * Portions are Copyright (C) 1998 Netscape Communications Corporation.
 *
 * Other contributors:
 *   Robert O'Callahan <roc+@cs.cmu.edu>
 *   David Baron <dbaron@fas.harvard.edu>
 *   Christian Biesinger <cbiesinger@web.de>
 *   Randall Jesup <rjesup@wgate.com>
 *   Roland Mainz <roland.mainz@informatik.med.uni-giessen.de>
 *   Josh Soref <timeless@mac.com>
 *   Boris Zbarsky <bzbarsky@mit.edu>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * Alternatively, the contents of this file may be used under the terms
 * of either the Mozilla Public License Version 1.1, found at
 * http://www.mozilla.org/MPL/ (the "MPL") or the GNU General Public
 * License Version 2.0, found at http://www.fsf.org/copyleft/gpl.html
 * (the "GPL"), in which case the provisions of the MPL or the GPL are
 * applicable instead of those above.  If you wish to allow use of your
 * version of this file only under the terms of one of those two
 * licenses (the MPL or the GPL) and not to allow others to use your
 * version of this file under the LGPL, indicate your decision by
 * deletingthe provisions above and replace them with the notice and
 * other provisions required by the MPL or the GPL, as the case may be.
 * If you do not delete the provisions above, a recipient may use your
 * version of this file under any of the LGPL, the MPL or the GPL.
 */

#pragma once

#include "Krystal.Booey/Graphics/Geometry/LayoutRect.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/Types/Maybe.hpp"

namespace krys::boo
{
  /// @brief How to align a rect when scrolling it into view, depending on whether it's already visible,
  /// partly visible or hidden.
  struct ScrollAlignment
  {
    enum class Behavior : uint8
    {
      NoScroll,
      AlignCenter,
      AlignTop,
      AlignBottom,
      AlignLeft,
      AlignRight,
      AlignToClosestEdge
    };

    Behavior RectVisible;
    Behavior RectHidden;
    Behavior RectPartial;
    bool EnableLegacyHorizontalVisibilityThreshold {true};

    KRYS_NODISCARD Behavior GetVisibleBehavior() const noexcept
    {
      return RectVisible;
    }
    KRYS_NODISCARD Behavior GetPartialBehavior() const noexcept
    {
      return RectPartial;
    }
    KRYS_NODISCARD Behavior GetHiddenBehavior() const noexcept
    {
      return RectHidden;
    }

    void DisableLegacyHorizontalVisibilityThreshold() noexcept
    {
      EnableLegacyHorizontalVisibilityThreshold = false;
    }
    KRYS_NODISCARD bool LegacyHorizontalVisibilityThresholdEnabled() const noexcept
    {
      return EnableLegacyHorizontalVisibilityThreshold;
    }

    static const ScrollAlignment NoScroll;
    static const ScrollAlignment AlignCenterIfNotVisible;
    static const ScrollAlignment AlignToEdgeIfNotVisible;
    static const ScrollAlignment AlignCenterIfNeeded;
    static const ScrollAlignment AlignToEdgeIfNeeded;
    static const ScrollAlignment AlignCenterAlways;
    static const ScrollAlignment AlignTopAlways;
    static const ScrollAlignment AlignRightAlways;
    static const ScrollAlignment AlignLeftAlways;
    static const ScrollAlignment AlignBottomAlways;
  };

  /// @brief The part of a scroller's contents to make visible so @p exposeRect is aligned as asked, as a rect
  /// with the size of @p visibleBounds.
  KRYS_NODISCARD LayoutRect GetRectToExposeForScrollIntoView(
    const LayoutRect &visibleBounds, const LayoutRect &exposeRect, const ScrollAlignment &alignX,
    const ScrollAlignment &alignY, const Maybe<LayoutRect> visibilityCheckRect = null) noexcept;
}
