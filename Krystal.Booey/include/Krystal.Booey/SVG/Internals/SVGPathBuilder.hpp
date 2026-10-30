#pragma once

#include "Krystal.Booey/Graphics/FloatPoint.hpp"
#include "Krystal.Booey/SVG/Internals/SVGPathConsumer.hpp"
#include "Krystal.Core/Assert.hpp"

namespace krys::boo
{
  class Path;
}

namespace krys::boo::svg
{
  /// @brief Builds a `Path` from parsed SVG path data (in unaltered or normalised parsing mode).
  class SVGPathBuilder final : public SVGPathConsumer
  {
  private:
    Path &_path; // FIXME: This should use a WeakRef.
    FloatPoint _current;

  public:
    explicit SVGPathBuilder(Path &) noexcept;

  private:
    void IncrementPathSegmentCount() noexcept final
    {
    }
    KRYS_NODISCARD bool ContinueConsuming() noexcept final
    {
      return true;
    }

    // Used in UnalteredParsing/NormalizedParsing modes.
    void MoveTo(const FloatPoint &, bool closed, PathCoordinateMode) noexcept final;
    void LineTo(const FloatPoint &, PathCoordinateMode) noexcept final;
    void CurveToCubic(const FloatPoint &, const FloatPoint &, const FloatPoint &,
                      PathCoordinateMode) noexcept final;
    void ClosePath() noexcept final;

    // Only used in UnalteredParsing mode.
    void LineToHorizontal(float, PathCoordinateMode) noexcept final
    {
      krys_debug_assert_msg(false, "Unreachable");
    }
    void LineToVertical(float, PathCoordinateMode) noexcept final
    {
      krys_debug_assert_msg(false, "Unreachable");
    }
    void CurveToCubicSmooth(const FloatPoint &, const FloatPoint &, PathCoordinateMode) noexcept final
    {
      krys_debug_assert_msg(false, "Unreachable");
    }
    void CurveToQuadratic(const FloatPoint &, const FloatPoint &, PathCoordinateMode) noexcept final
    {
      krys_debug_assert_msg(false, "Unreachable");
    }
    void CurveToQuadraticSmooth(const FloatPoint &, PathCoordinateMode) noexcept final
    {
      krys_debug_assert_msg(false, "Unreachable");
    }
    void ArcTo(float, float, float, bool, bool, const FloatPoint &, PathCoordinateMode) noexcept final
    {
      krys_debug_assert_msg(false, "Unreachable");
    }
  };
}
