/*
 * Copyright (C) 2016 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

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
