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
#include "Krystal.Core/Types/Span.hpp"
#include <cstring>
#include <type_traits>

namespace krys::boo::svg
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
    KRYS_NODISCARD Maybe<MoveToSegment> ParseMoveToSegment(FloatPoint currentPoint) noexcept final;
    KRYS_NODISCARD Maybe<LineToSegment> ParseLineToSegment(FloatPoint currentPoint) noexcept final;
    KRYS_NODISCARD Maybe<LineToHorizontalSegment>
      ParseLineToHorizontalSegment(FloatPoint currentPoint) noexcept final;
    KRYS_NODISCARD Maybe<LineToVerticalSegment>
      ParseLineToVerticalSegment(FloatPoint currentPoint) noexcept final;
    KRYS_NODISCARD Maybe<CurveToCubicSegment>
      ParseCurveToCubicSegment(FloatPoint currentPoint) noexcept final;
    KRYS_NODISCARD Maybe<CurveToCubicSmoothSegment>
      ParseCurveToCubicSmoothSegment(FloatPoint currentPoint) noexcept final;
    KRYS_NODISCARD Maybe<CurveToQuadraticSegment>
      ParseCurveToQuadraticSegment(FloatPoint currentPoint) noexcept final;
    KRYS_NODISCARD Maybe<CurveToQuadraticSmoothSegment>
      ParseCurveToQuadraticSmoothSegment(FloatPoint currentPoint) noexcept final;
    KRYS_NODISCARD Maybe<ArcToSegment> ParseArcToSegment(FloatPoint currentPoint) noexcept final;

    template <typename TData>
    KRYS_NODISCARD TData ReadType() noexcept
    {
      static_assert(std::is_trivially_copyable_v<TData>);
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
      static_assert(std::is_same_v<std::underlying_type_t<SVGPathSegType>, uint8>);
      return static_cast<SVGPathSegType>(ReadType<uint8>());
    }

    KRYS_NODISCARD FloatPoint ReadFloatPoint() noexcept
    {
      return ReadType<FloatPoint>();
    }
  };
}
