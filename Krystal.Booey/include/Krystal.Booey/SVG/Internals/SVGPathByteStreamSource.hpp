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
