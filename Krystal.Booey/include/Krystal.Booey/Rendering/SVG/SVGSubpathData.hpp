/*
 * Copyright (C) 2012 Google, Inc.
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
 */

#pragma once

#include "Krystal.Booey/Graphics/Paths/Path.hpp"
#include "Krystal.Core/Types/List.hpp"

namespace krys::boo
{
  /// @brief Finds a path's zero length subpaths, which round and square caps draw as dots.
  class SVGSubpathData
  {
  private:
    List<FloatPoint> &_zeroLengthSubpathLocations;
    FloatPoint _lastPoint;
    FloatPoint _movePoint;
    bool _haveSeenMoveOnly {false};
    bool _pathIsZeroLength {false};

  public:
    explicit SVGSubpathData(List<FloatPoint> &zeroLengthSubpathLocations) noexcept
        : _zeroLengthSubpathLocations(zeroLengthSubpathLocations)
    {
    }

    static void UpdateFromPathElement(SVGSubpathData &subpathFinder, const PathElement &element) noexcept
    {
      switch (element.ElementType)
      {
        case PathElement::Type::MoveToPoint:
          if (subpathFinder._pathIsZeroLength && !subpathFinder._haveSeenMoveOnly)
          {
            subpathFinder._zeroLengthSubpathLocations.push_back(subpathFinder._lastPoint);
          }
          subpathFinder._lastPoint = subpathFinder._movePoint = element.Points[0];
          subpathFinder._haveSeenMoveOnly = true;
          subpathFinder._pathIsZeroLength = true;
          break;
        case PathElement::Type::AddLineToPoint:
          if (subpathFinder._lastPoint != element.Points[0])
          {
            subpathFinder._pathIsZeroLength = false;
            subpathFinder._lastPoint = element.Points[0];
          }
          subpathFinder._haveSeenMoveOnly = false;
          break;
        case PathElement::Type::AddQuadCurveToPoint:
          if (subpathFinder._lastPoint != element.Points[0] || element.Points[0] != element.Points[1])
          {
            subpathFinder._pathIsZeroLength = false;
            subpathFinder._lastPoint = element.Points[1];
          }
          subpathFinder._haveSeenMoveOnly = false;
          break;
        case PathElement::Type::AddCurveToPoint:
          if (subpathFinder._lastPoint != element.Points[0] || element.Points[0] != element.Points[1]
              || element.Points[1] != element.Points[2])
          {
            subpathFinder._pathIsZeroLength = false;
            subpathFinder._lastPoint = element.Points[2];
          }
          subpathFinder._haveSeenMoveOnly = false;
          break;
        case PathElement::Type::CloseSubpath:
          if (subpathFinder._pathIsZeroLength)
          {
            subpathFinder._zeroLengthSubpathLocations.push_back(subpathFinder._lastPoint);
          }
          subpathFinder._haveSeenMoveOnly = true; // This is an implicit move for the next element
          subpathFinder._pathIsZeroLength = true; // A new sub-path also starts here
          subpathFinder._lastPoint = subpathFinder._movePoint;
          break;
      }
    }

    void PathIsDone() noexcept
    {
      if (_pathIsZeroLength && !_haveSeenMoveOnly)
      {
        _zeroLengthSubpathLocations.push_back(_lastPoint);
      }
    }
  };
}
