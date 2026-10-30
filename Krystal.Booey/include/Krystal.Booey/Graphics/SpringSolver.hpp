#pragma once

#include "Krystal.Core/Attributes.hpp"
#include <cmath>

namespace krys::boo
{
  /// @brief Solves a damped spring from 0 to 1 over time.
  class SpringSolver
  {
  private:
    double _w0 {0};
    double _zeta {0};
    double _wd {0};
    double _a {0};
    double _b {0};

  public:
    SpringSolver(double mass, double stiffness, double damping, double initialVelocity) noexcept
    {
      _w0 = std::sqrt(stiffness / mass);
      _zeta = damping / (2 * std::sqrt(stiffness * mass));

      if (_zeta < 1)
      {
        // Under-damped.
        _wd = _w0 * std::sqrt(1 - _zeta * _zeta);
        _a = 1;
        _b = (_zeta * _w0 + -initialVelocity) / _wd;
      }
      else
      {
        // Critically damped (ignoring over-damped case for now).
        _a = 1;
        _b = -initialVelocity + _w0;
      }
    }

    KRYS_NODISCARD double Solve(double t) const noexcept
    {
      if (_zeta < 1)
      {
        // Under-damped
        t = std::exp(-t * _zeta * _w0) * (_a * std::cos(_wd * t) + _b * std::sin(_wd * t));
      }
      else
      {
        // Critically damped (ignoring over-damped case for now).
        t = (_a + _b * t) * std::exp(-t * _w0);
      }

      // Map range from [1..0] to [0..1].
      return 1 - t;
    }
  };
}
