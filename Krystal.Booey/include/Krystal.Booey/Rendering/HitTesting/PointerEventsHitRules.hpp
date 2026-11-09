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

#pragma once

#include "Krystal.Booey/Rendering/Style/RenderStyleConstants.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Numeric.hpp"

namespace krys::boo
{
  class HitTestRequest;

  enum class PointerEventsHitTestingTargetType : uint8
  {
    SVGImage,
    SVGPath,
    SVGText
  };

  /// @brief What `pointer-events` lets SVG graphics be hit by: their fill, stroke or bounding box, and
  /// whether they must be visible or painted.
  class PointerEventsHitRules
  {
  public:
    using HitTestingTargetType = PointerEventsHitTestingTargetType;

    PointerEventsHitRules(HitTestingTargetType, const HitTestRequest &, PointerEvents) noexcept;

    bool RequireVisible {false};
    bool RequireFill {false};
    bool RequireStroke {false};
    bool CanHitStroke {false};
    bool CanHitFill {false};
    bool CanHitBoundingBox {false};
  };
}
