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

#include "Krystal.Booey/SVG/Internals/SVGPathByteStream.hpp"
#include "Krystal.Booey/SVG/Internals/SVGPathConsumer.hpp"
#include "Krystal.Booey/SVG/Internals/SVGPathSegType.hpp"
#include "Krystal.Core/Types/Span.hpp"
#include <type_traits>

namespace krys::boo::svg
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
    void MoveTo(const FloatPoint &targetPoint, bool closed, PathCoordinateMode mode) noexcept final;
    void LineTo(const FloatPoint &targetPoint, PathCoordinateMode mode) noexcept final;
    void CurveToCubic(const FloatPoint &point1, const FloatPoint &point2, const FloatPoint &targetPoint,
                      PathCoordinateMode mode) noexcept final;
    void ClosePath() noexcept final;

    // Only used in UnalteredParsing mode.
    void LineToHorizontal(float x, PathCoordinateMode mode) noexcept final;
    void LineToVertical(float y, PathCoordinateMode mode) noexcept final;
    void CurveToCubicSmooth(const FloatPoint &point2, const FloatPoint &targetPoint,
                            PathCoordinateMode mode) noexcept final;
    void CurveToQuadratic(const FloatPoint &point1, const FloatPoint &targetPoint,
                          PathCoordinateMode mode) noexcept final;
    void CurveToQuadraticSmooth(const FloatPoint &targetPoint, PathCoordinateMode mode) noexcept final;
    void ArcTo(float r1, float r2, float angle, bool largeArcFlag, bool sweepFlag,
               const FloatPoint &targetPoint, PathCoordinateMode mode) noexcept final;

    template <typename TData>
    void WriteType(const TData &data) noexcept
    {
      static_assert(std::is_trivially_copyable_v<TData>);
      _byteStream.Append(Span<const uint8>(reinterpret_cast<const uint8 *>(&data), sizeof(TData)));
    }

    void WriteSegmentType(SVGPathSegType type) noexcept
    {
      static_assert(std::is_same_v<std::underlying_type_t<SVGPathSegType>, uint8>);
      _byteStream.Append(static_cast<uint8>(type));
    }
  };
}
