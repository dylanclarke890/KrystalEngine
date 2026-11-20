/*
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

#pragma once

#include "Krystal.Booey/SVG/Internals/SVGPathByteStream.hpp"
#include "Krystal.Booey/SVG/Internals/SVGPathSource.hpp"
#include "Krystal.Core/Assert.hpp"
#include "Krystal.Core/Concepts.hpp"
#include "Krystal.Core/Types/Span.hpp"
#include <cstring>

namespace krys::boo
{
  /// @brief Reads the segments back out of an `SVGPathByteStream`.
  class SVGPathByteStreamSource final : public SVGPathSource
  {
  private:
    Span<const uint8> _streamCurrent;

  public:
    explicit SVGPathByteStreamSource(const SVGPathByteStream &stream) noexcept;

  private:
    KRYS_NODISCARD bool HasMoreData() const noexcept final;

    bool MoveToNextToken() noexcept final
    {
      return true;
    }

    KRYS_NODISCARD SVGPathSegType NextCommand(SVGPathSegType previousCommand) noexcept final;

    KRYS_NODISCARD Maybe<SVGPathSegType> ParseSVGSegmentType() noexcept final;

    KRYS_NODISCARD Maybe<MoveToSegment> ParseMoveToSegment(PointF currentPoint) noexcept final;

    KRYS_NODISCARD Maybe<LineToSegment> ParseLineToSegment(PointF currentPoint) noexcept final;

    KRYS_NODISCARD Maybe<LineToHorizontalSegment>
      ParseLineToHorizontalSegment(PointF currentPoint) noexcept final;

    KRYS_NODISCARD Maybe<LineToVerticalSegment>
      ParseLineToVerticalSegment(PointF currentPoint) noexcept final;

    KRYS_NODISCARD Maybe<CurveToCubicSegment> ParseCurveToCubicSegment(PointF currentPoint) noexcept final;

    KRYS_NODISCARD Maybe<CurveToCubicSmoothSegment>
      ParseCurveToCubicSmoothSegment(PointF currentPoint) noexcept final;

    KRYS_NODISCARD Maybe<CurveToQuadraticSegment>
      ParseCurveToQuadraticSegment(PointF currentPoint) noexcept final;

    KRYS_NODISCARD Maybe<CurveToQuadraticSmoothSegment>
      ParseCurveToQuadraticSmoothSegment(PointF currentPoint) noexcept final;

    KRYS_NODISCARD Maybe<ArcToSegment> ParseArcToSegment(PointF currentPoint) noexcept final;

    template <typename TData>
    KRYS_NODISCARD TData ReadType() noexcept
    {
      static_assert(TriviallyCopyable<TData>);
      krys_assert(_streamCurrent.size() >= sizeof(TData));

      TData data;
      std::memcpy(&data, _streamCurrent.data(), sizeof(TData));
      _streamCurrent = _streamCurrent.subspan(sizeof(TData));
      return data;
    }

    KRYS_NODISCARD bool ReadFlag() noexcept
    {
      return ReadType<bool>();
    }

    KRYS_NODISCARD float ReadFloat() noexcept
    {
      return ReadType<float>();
    }

    KRYS_NODISCARD SVGPathSegType ReadSVGSegmentType() noexcept
    {
      static_assert(SameType<underlying_t<SVGPathSegType>, uint8>);
      return static_cast<SVGPathSegType>(ReadType<uint8>());
    }

    KRYS_NODISCARD PointF ReadPointF() noexcept
    {
      return ReadType<PointF>();
    }
  };
}
