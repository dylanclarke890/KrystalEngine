/*
 * Copyright (C) 2004, 2005, 2006, 2007, 2008 Nikolas Zimmermann <zimmermann@kde.org>
 * Copyright (C) 2004, 2005 Rob Buis <buis@kde.org>
 * Copyright (C) 2007 Eric Seidel <eric@webkit.org>
 * Copyright (C) 2015 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/SVG/Internals/SVGPathTraversalStateBuilder.hpp"
#include "Krystal.Booey/Graphics/Paths/PathTraversalState.hpp"

namespace krys::boo::svg
{
  SVGPathTraversalStateBuilder::SVGPathTraversalStateBuilder(PathTraversalState &state,
                                                             float desiredLength) noexcept
      : _traversalState(state)
  {
    _traversalState.SetDesiredLength(desiredLength);
  }

  void SVGPathTraversalStateBuilder::MoveTo(const FloatPoint &targetPoint, bool, PathCoordinateMode) noexcept
  {
    (void)_traversalState.ProcessPathElement(PathElement::Type::MoveToPoint, std::span {&targetPoint, 1});
  }

  void SVGPathTraversalStateBuilder::LineTo(const FloatPoint &targetPoint, PathCoordinateMode) noexcept
  {
    (void)_traversalState.ProcessPathElement(PathElement::Type::AddLineToPoint, std::span {&targetPoint, 1});
  }

  void SVGPathTraversalStateBuilder::CurveToCubic(const FloatPoint &point1, const FloatPoint &point2,
                                                  const FloatPoint &targetPoint, PathCoordinateMode) noexcept
  {
    std::array points {point1, point2, targetPoint};

    (void)_traversalState.ProcessPathElement(PathElement::Type::AddCurveToPoint,
                                             std::span<const FloatPoint> {points});
  }

  void SVGPathTraversalStateBuilder::ClosePath() noexcept
  {
    (void)_traversalState.ProcessPathElement(PathElement::Type::CloseSubpath, {});
  }

  bool SVGPathTraversalStateBuilder::ContinueConsuming() noexcept
  {
    return !_traversalState.Success();
  }

  float SVGPathTraversalStateBuilder::TotalLength() const noexcept
  {
    return _traversalState.TotalLength();
  }

  FloatPoint SVGPathTraversalStateBuilder::CurrentPoint() const noexcept
  {
    return _traversalState.Current();
  }
}
