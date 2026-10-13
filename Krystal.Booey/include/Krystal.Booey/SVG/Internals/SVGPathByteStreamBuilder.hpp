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
