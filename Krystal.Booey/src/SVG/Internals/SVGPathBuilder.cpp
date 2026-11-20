/*
 * Copyright (C) 2002, 2003 The Karbon Developers
 * Copyright (C) 2006 Alexander Kellett <lypanov@kde.org>
 * Copyright (C) 2006, 2007 Rob Buis <buis@kde.org>
 * Copyright (C) 2007, 2009, 2015 Apple Inc. All rights reserved.
 * Copyright (C) Research In Motion Limited 2010. All rights reserved.
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

#include "Krystal.Booey/SVG/Internals/SVGPathBuilder.hpp"
#include "Krystal.Booey/Graphics/Paths/Path.hpp"

namespace krys::boo
{
  SVGPathBuilder::SVGPathBuilder(Path &path) noexcept : _path(path)
  {
  }

  void SVGPathBuilder::MoveTo(const PointF &targetPoint, bool closed, svg::PathCoordinateMode mode) noexcept
  {
    _current = mode == svg::PathCoordinateMode::AbsoluteCoordinates ? targetPoint : _current + targetPoint;
    if (closed && !_path.IsEmpty())
    {
      _path.CloseSubpath();
    }
    _path.MoveTo(_current);
  }

  void SVGPathBuilder::LineTo(const PointF &targetPoint, svg::PathCoordinateMode mode) noexcept
  {
    _current = mode == svg::PathCoordinateMode::AbsoluteCoordinates ? targetPoint : _current + targetPoint;
    _path.AddLineTo(_current);
  }

  void SVGPathBuilder::CurveToCubic(const PointF &point1, const PointF &point2, const PointF &targetPoint,
                                    svg::PathCoordinateMode mode) noexcept
  {
    if (mode == svg::PathCoordinateMode::RelativeCoordinates)
    {
      _path.AddBezierCurveTo(_current + point1, _current + point2, _current + targetPoint);
      _current += targetPoint;
    }
    else
    {
      _current = targetPoint;
      _path.AddBezierCurveTo(point1, point2, _current);
    }
  }

  void SVGPathBuilder::ClosePath() noexcept
  {
    _path.CloseSubpath();
  }
}
