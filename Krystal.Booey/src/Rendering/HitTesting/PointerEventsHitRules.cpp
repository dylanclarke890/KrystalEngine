/*
    Copyright (C) 2007 Rob Buis <buis@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    aint with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "Krystal.Booey/Rendering/HitTesting/PointerEventsHitRules.hpp"
#include "Krystal.Booey/Rendering/HitTesting/HitTestRequest.hpp"

namespace krys::boo
{
  PointerEventsHitRules::PointerEventsHitRules(HitTestingTargetType hitTestingTargetType,
                                               const HitTestRequest &request,
                                               PointerEvents pointerEvents) noexcept
  {
    if (request.SVGClipContent())
    {
      pointerEvents = PointerEvents::Fill;
    }

    if (hitTestingTargetType == HitTestingTargetType::SVGPath)
    {
      switch (pointerEvents)
      {
        case PointerEvents::VisiblePainted:
        case PointerEvents::Auto: // "auto" is like "visiblePainted" when in SVG content
          RequireFill = true;
          RequireStroke = true;
          [[fallthrough]];
        case PointerEvents::Visible:
          RequireVisible = true;
          CanHitFill = true;
          CanHitStroke = true;
          break;
        case PointerEvents::VisibleFill:
          RequireVisible = true;
          CanHitFill = true;
          break;
        case PointerEvents::VisibleStroke:
          RequireVisible = true;
          CanHitStroke = true;
          break;
        case PointerEvents::Painted:
          RequireFill = true;
          RequireStroke = true;
          [[fallthrough]];
        case PointerEvents::All:
          CanHitFill = true;
          CanHitStroke = true;
          break;
        case PointerEvents::Fill:   CanHitFill = true; break;
        case PointerEvents::Stroke: CanHitStroke = true; break;
        case PointerEvents::BoundingBox:
          CanHitFill = true;
          CanHitBoundingBox = true;
          break;
        case PointerEvents::None:
          // nothing to do here, defaults are all false.
          break;
      }
    }
    else
    {
      switch (pointerEvents)
      {
        case PointerEvents::VisiblePainted:
        case PointerEvents::Auto: // "auto" is like "visiblePainted" when in SVG content
          RequireVisible = true;
          RequireFill = true;
          RequireStroke = true;
          CanHitFill = true;
          CanHitStroke = true;
          break;
        case PointerEvents::VisibleFill:
        case PointerEvents::VisibleStroke:
        case PointerEvents::Visible:
          RequireVisible = true;
          CanHitFill = true;
          CanHitStroke = true;
          break;
        case PointerEvents::Painted:
          RequireFill = true;
          RequireStroke = true;
          CanHitFill = true;
          CanHitStroke = true;
          break;
        case PointerEvents::Fill:
        case PointerEvents::Stroke:
        case PointerEvents::All:
          CanHitFill = true;
          CanHitStroke = true;
          break;
        case PointerEvents::BoundingBox:
          CanHitFill = true;
          CanHitBoundingBox = true;
          break;
        case PointerEvents::None:
          // nothing to do here, defaults are all false.
          break;
      }
    }
  }
}
