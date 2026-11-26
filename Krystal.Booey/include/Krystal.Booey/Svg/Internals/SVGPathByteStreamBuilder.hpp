/*
 * Copyright (C) Research In Motion Limited 2010. All rights reserved.
 * Copyright (C) 2015 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/Svg/Internals/SVGPathByteStream.hpp"
#include "Krystal.Booey/Svg/Internals/SVGPathConsumer.hpp"
#include "Krystal.Booey/Svg/Internals/SVGPathSegType.hpp"
#include "Krystal.Core/Concepts.hpp"
#include "Krystal.Core/Containers/Span.hpp"

namespace krys::boo
{
  /// @brief Writes the segments it consumes to an `SVGPathByteStream`.
  class SVGPathByteStreamBuilder final : public SVGPathConsumer
  {
  private:
    SVGPathByteStream &_byteStream;

  public:
    explicit SVGPathByteStreamBuilder(SVGPathByteStream &byteStream) noexcept;

  private:
    void IncrementPathSegmentCount() noexcept final
    {
    }

    KRYS_NODISCARD bool ContinueConsuming() noexcept final
    {
      return true;
    }

    // Used in UnalteredParsing/NormalizedParsing modes.
    void MoveTo(const PointF &targetPoint, bool closed, svg::PathCoordinateMode mode) noexcept final;

    void LineTo(const PointF &targetPoint, svg::PathCoordinateMode mode) noexcept final;

    void CurveToCubic(const PointF &point1, const PointF &point2, const PointF &targetPoint,
                      svg::PathCoordinateMode mode) noexcept final;

    void ClosePath() noexcept final;

    // Only used in UnalteredParsing mode.
    void LineToHorizontal(float x, svg::PathCoordinateMode mode) noexcept final;

    void LineToVertical(float y, svg::PathCoordinateMode mode) noexcept final;

    void CurveToCubicSmooth(const PointF &point2, const PointF &targetPoint,
                            svg::PathCoordinateMode mode) noexcept final;

    void CurveToQuadratic(const PointF &point1, const PointF &targetPoint,
                          svg::PathCoordinateMode mode) noexcept final;

    void CurveToQuadraticSmooth(const PointF &targetPoint, svg::PathCoordinateMode mode) noexcept final;

    void ArcTo(float r1, float r2, float angle, bool largeArcFlag, bool sweepFlag, const PointF &targetPoint,
               svg::PathCoordinateMode mode) noexcept final;

    template <typename TData>
    void WriteType(const TData &data) noexcept
    {
      static_assert(TriviallyCopyable<TData>);
      _byteStream.Append(Span<const uint8>(reinterpret_cast<const uint8 *>(&data), sizeof(TData)));
    }

    void WriteSegmentType(SVGPathSegType type) noexcept
    {
      static_assert(SameType<underlying_t<SVGPathSegType>, uint8>);
      _byteStream.Append(static_cast<uint8>(type));
    }
  };
}
