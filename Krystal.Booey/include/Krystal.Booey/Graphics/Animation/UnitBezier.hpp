#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/Array.hpp"
#include <algorithm>
#include <cmath>

namespace krys::boo
{
  /// @brief A cubic Bézier curve from (0, 0) to (1, 1) with two control points, solved for y given x.
  struct UnitBezier
  {
  private:
    constexpr static int CubicBezierSplineSamples = 11;

    double _ax;
    double _bx;
    double _cx;

    double _ay;
    double _by;
    double _cy;

    double _startGradient;
    double _endGradient;

    Array<double, CubicBezierSplineSamples> _splineSamples;

  public:
    constexpr static double BezierEpsilon = 1e-7;
    constexpr static int MaxNewtonIterations = 4;

    UnitBezier(double p1x, double p1y, double p2x, double p2y) noexcept
    {
      // Calculate the polynomial coefficients, implicit first and last control points are (0,0) and (1,1).
      _cx = 3.0 * p1x;
      _bx = 3.0 * (p2x - p1x) - _cx;
      _ax = 1.0 - _cx - _bx;

      _cy = 3.0 * p1y;
      _by = 3.0 * (p2y - p1y) - _cy;
      _ay = 1.0 - _cy - _by;

      // End-point gradients are used to calculate timing function results
      // outside the range [0, 1].
      //
      // There are four possibilities for the gradient at each end:
      // (1) the closest control point is not horizontally coincident with regard to
      //     (0, 0) or (1, 1). In this case the line between the end point and
      //     the control point is tangent to the bezier at the end point.
      // (2) the closest control point is coincident with the end point. In
      //     this case the line between the end point and the far control
      //     point is tangent to the bezier at the end point.
      // (3) both internal control points are coincident with an endpoint. There
      //     are two special case that fall into this category:
      //     CubicBezier(0, 0, 0, 0) and CubicBezier(1, 1, 1, 1). Both are
      //     equivalent to linear.
      // (4) the closest control point is horizontally coincident with the end
      //     point, but vertically distinct. In this case the gradient at the
      //     end point is Infinite. However, this causes issues when
      //     interpolating. As a result, we break down to a simple case of
      //     0 gradient under these conditions.
      if (p1x > 0)
      {
        _startGradient = p1y / p1x;
      }
      else if (!p1y && p2x > 0)
      {
        _startGradient = p2y / p2x;
      }
      else if (!p1y && !p2y)
      {
        _startGradient = 1;
      }
      else
      {
        _startGradient = 0;
      }
      if (p2x < 1)
      {
        _endGradient = (p2y - 1) / (p2x - 1);
      }
      else if (p2y == 1 && p1x < 1)
      {
        _endGradient = (p1y - 1) / (p1x - 1);
      }
      else if (p2y == 1 && p1y == 1)
      {
        _endGradient = 1;
      }
      else
      {
        _endGradient = 0;
      }

      double deltaT = 1.0 / (CubicBezierSplineSamples - 1);
      for (int i = 0; i < CubicBezierSplineSamples; i++)
      {
        _splineSamples[i] = SampleCurveX(i * deltaT);
      }
    }

    KRYS_NODISCARD double SampleCurveX(double t) const noexcept
    {
      // `ax t^3 + bx t^2 + cx t' expanded using Horner's rule.
      return ((_ax * t + _bx) * t + _cx) * t;
    }

    KRYS_NODISCARD double SampleCurveY(double t) const noexcept
    {
      return ((_ay * t + _by) * t + _cy) * t;
    }

    KRYS_NODISCARD double SampleCurveDerivativeX(double t) const noexcept
    {
      return (3.0 * _ax * t + 2.0 * _bx) * t + _cx;
    }

    /// @brief Given an x value, find a parametric value it came from.
    KRYS_NODISCARD double SolveCurveX(double x, double epsilon) const noexcept
    {
      double t0 = 0.0;
      double t1 = 0.0;
      double t2 = x;
      double x2 = 0.0;
      double d2 = 0.0;
      int i = 0;

      // Linear interpolation of spline curve for initial guess.
      double deltaT = 1.0 / (CubicBezierSplineSamples - 1);
      for (i = 1; i < CubicBezierSplineSamples; i++)
      {
        if (x <= _splineSamples[i])
        {
          t1 = deltaT * i;
          t0 = t1 - deltaT;
          t2 = t0 + (t1 - t0) * (x - _splineSamples[i - 1]) / (_splineSamples[i] - _splineSamples[i - 1]);
          break;
        }
      }

      // Perform a few iterations of Newton's method -- normally very fast.
      // See https://en.wikipedia.org/wiki/Newton%27s_method.
      double newtonEpsilon = std::min(BezierEpsilon, epsilon);
      for (i = 0; i < MaxNewtonIterations; i++)
      {
        x2 = SampleCurveX(t2) - x;
        if (std::abs(x2) < newtonEpsilon)
        {
          return t2;
        }
        d2 = SampleCurveDerivativeX(t2);
        if (std::abs(d2) < BezierEpsilon)
        {
          break;
        }
        t2 = t2 - x2 / d2;
      }
      if (std::abs(x2) < epsilon)
      {
        return t2;
      }

      // Fall back to the bisection method for reliability.
      while (t0 < t1)
      {
        x2 = SampleCurveX(t2);
        if (std::abs(x2 - x) < epsilon)
        {
          return t2;
        }
        if (x > x2)
        {
          t0 = t2;
        }
        else
        {
          t1 = t2;
        }
        t2 = (t1 + t0) * .5;
      }

      // Failure.
      return t2;
    }

    KRYS_NODISCARD double Solve(double x, double epsilon) const noexcept
    {
      if (x < 0.0)
      {
        return 0.0 + _startGradient * x;
      }
      if (x > 1.0)
      {
        return 1.0 + _endGradient * (x - 1.0);
      }
      return SampleCurveY(SolveCurveX(x, epsilon));
    }
  };
}
