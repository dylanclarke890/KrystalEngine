/*
   Copyright (C) 2000 Simon Hausmann <hausmann@kde.org>
   Copyright (C) 2006 Apple Inc. All rights reserved.

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#pragma once

#include "Krystal.Booey/Platform/Events/PlatformMouseEvent.hpp"
#include "Krystal.Booey/Rendering/HitTesting/HitTestResult.hpp"
#include "Krystal.Core/Attributes.hpp"

namespace krys::boo
{
  /// @brief A mouse event with what it hit.
  class MouseEventWithHitTestResults
  {
  private:
    PlatformMouseEvent _event;
    HitTestResult _hitTestResult;

  public:
    MouseEventWithHitTestResults(const PlatformMouseEvent &event, const HitTestResult &hitTestResult) noexcept
        : _event(event), _hitTestResult(hitTestResult)
    {
    }

    KRYS_NODISCARD const PlatformMouseEvent &Event() const noexcept
    {
      return _event;
    }

    KRYS_NODISCARD const HitTestResult &GetHitTestResult() const noexcept
    {
      return _hitTestResult;
    }

    KRYS_NODISCARD LayoutPoint LocalPoint() const noexcept
    {
      return _hitTestResult.LocalPoint();
    }

    KRYS_NODISCARD bool IsOverLink() const noexcept
    {
      return _hitTestResult.IsOverLink();
    }

    KRYS_NODISCARD dom::Node *TargetNode() const noexcept
    {
      return _hitTestResult.TargetNode();
    }

    KRYS_NODISCARD Scrollbar *GetScrollbar() const noexcept
    {
      return _hitTestResult.GetScrollbar();
    }
  };
}
