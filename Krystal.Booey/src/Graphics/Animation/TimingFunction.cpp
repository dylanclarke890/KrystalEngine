/*
 * Copyright (C) 2015 Apple Inc. All rights reserved.
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
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "Krystal.Booey/Graphics/Animation/TimingFunction.hpp"
#include "Krystal.Booey/CSS/Values/NumberFormatting.hpp"
#include "Krystal.Core/Maths/SpringSolver.hpp"
#include "Krystal.Core/Maths/UnitBezier.hpp"
#include "Krystal.Core/Strings/StringBuilder.hpp"
#include "Krystal.Core/Strings/TextStream.hpp"
#include <algorithm>
#include <cmath>
#include <limits>

namespace krys::boo
{
  namespace
  {
    // The text of a timing function, written as its CSS function.
    void AppendFunctionText(StringBuilder &ts, const TimingFunction &timingFunction) noexcept
    {
      switch (timingFunction.GetType())
      {
        case TimingFunction::Type::LinearFunction:
        {
          const auto &function = Downcast<LinearTimingFunction>(timingFunction);
          ts.Append(u8"linear(");
          for (size_t i = 0; i < function.Points().size(); ++i)
          {
            if (i)
            {
              ts.Append(u8", ");
            }

            const auto &point = function.Points()[i];
            FormatCSSNumber(ts, point.Value);
            ts.Append(u8' ');
            FormatCSSNumber(ts, point.Progress * 100.0);
            ts.Append(u8'%');
          }
          ts.Append(u8')');
          break;
        }
        case TimingFunction::Type::CubicBezierFunction:
        {
          const auto &function = Downcast<CubicBezierTimingFunction>(timingFunction);
          ts.Append(u8"cubic-bezier(");
          FormatCSSNumber(ts, function.X1());
          ts.Append(u8", ");
          FormatCSSNumber(ts, function.Y1());
          ts.Append(u8", ");
          FormatCSSNumber(ts, function.X2());
          ts.Append(u8", ");
          FormatCSSNumber(ts, function.Y2());
          ts.Append(u8')');
          break;
        }
        case TimingFunction::Type::StepsFunction:
        {
          const auto &function = Downcast<StepsTimingFunction>(timingFunction);
          ts.Append(u8"steps(");
          FormatCSSNumber(ts, function.NumberOfSteps());
          if (auto stepPosition = function.GetStepPosition())
          {
            ts.Append(u8", ");
            switch (stepPosition.value())
            {
              case StepsTimingFunction::StepPosition::JumpStart: ts.Append(u8"jump-start"); break;
              case StepsTimingFunction::StepPosition::JumpEnd:   ts.Append(u8"jump-end"); break;
              case StepsTimingFunction::StepPosition::JumpNone:  ts.Append(u8"jump-none"); break;
              case StepsTimingFunction::StepPosition::JumpBoth:  ts.Append(u8"jump-both"); break;
              case StepsTimingFunction::StepPosition::Start:     ts.Append(u8"start"); break;
              case StepsTimingFunction::StepPosition::End:       ts.Append(u8"end"); break;
            }
          }
          ts.Append(u8')');
          break;
        }
        case TimingFunction::Type::SpringFunction:
        {
          const auto &function = Downcast<SpringTimingFunction>(timingFunction);
          ts.Append(u8"spring(");
          FormatCSSNumber(ts, function.Mass());
          ts.Append(u8' ');
          FormatCSSNumber(ts, function.Stiffness());
          ts.Append(u8' ');
          FormatCSSNumber(ts, function.Damping());
          ts.Append(u8' ');
          FormatCSSNumber(ts, function.InitialVelocity());
          ts.Append(u8')');
          break;
        }
      }
    }
  }

  double TimingFunction::TransformProgress(double progress, double duration, Before before) const noexcept
  {
    switch (GetType())
    {
      case Type::CubicBezierFunction:
      {
        const auto &function = Downcast<CubicBezierTimingFunction>(*this);
        if (function.IsLinear() || duration >= std::numeric_limits<double>::max())
        {
          return progress;
        }
        // The epsilon value we pass to UnitBezier::solve given that the animation is going to run over |dur|
        // seconds. The longer the animation, the more precision we need in the timing function result to
        // avoid ugly discontinuities.
        auto epsilon = 1.0 / (1000.0 * duration);
        return UnitBezier(function.X1(), function.Y1(), function.X2(), function.Y2())
          .Solve(progress, epsilon);
      }
      case Type::StepsFunction:
      {
        const auto &function = Downcast<StepsTimingFunction>(*this);
        return StepEasing(progress, function.NumberOfSteps(),
                          function.GetStepPosition().value_or(StepPosition::End), before == Before::Yes);
      }
      case Type::SpringFunction:
      {
        const auto &function = Downcast<SpringTimingFunction>(*this);
        return SpringSolver(function.Mass(), function.Stiffness(), function.Damping(),
                            function.InitialVelocity())
          .Solve(progress * duration);
      }
      case Type::LinearFunction:
      {
        return LinearEasing(Downcast<LinearTimingFunction>(*this).Points(), progress);
      }
    }

    krys_unreachable();
  }

  utf8_string TimingFunction::CssText() const noexcept
  {
    if (const auto *function = DynamicDowncast<LinearTimingFunction>(*this))
    {
      if (function->Points().empty())
      {
        return u8"linear";
      }
    }

    if (const auto *function = DynamicDowncast<CubicBezierTimingFunction>(*this))
    {
      if (function->X1() == 0.25 && function->Y1() == 0.1 && function->X2() == 0.25 && function->Y2() == 1.0)
      {
        return u8"ease";
      }
      if (function->X1() == 0.42 && !function->Y1() && function->X2() == 1.0 && function->Y2() == 1.0)
      {
        return u8"ease-in";
      }
      if (!function->X1() && !function->Y1() && function->X2() == 0.58 && function->Y2() == 1.0)
      {
        return u8"ease-out";
      }
      if (function->X1() == 0.42 && !function->Y1() && function->X2() == 0.58 && function->Y2() == 1.0)
      {
        return u8"ease-in-out";
      }
    }

    if (const auto *function = DynamicDowncast<StepsTimingFunction>(*this))
    {
      if (function->GetStepPosition() == StepsTimingFunction::StepPosition::JumpEnd
          || function->GetStepPosition() == StepsTimingFunction::StepPosition::End)
      {
        StringBuilder result;
        result.Append(u8"steps(");
        FormatCSSNumber(result, function->NumberOfSteps());
        result.Append(u8')');
        return krys::move(result).ToString();
      }
    }

    StringBuilder result;
    AppendFunctionText(result, *this);
    return krys::move(result).ToString();
  }

  bool LinearTimingFunction::Equals(const TimingFunction &other) const noexcept
  {
    const auto *otherLinear = DynamicDowncast<LinearTimingFunction>(other);
    return otherLinear && _points == otherLinear->_points;
  }

  Ref<CubicBezierTimingFunction> CubicBezierTimingFunction::Create(TimingFunctionPreset preset) noexcept
  {
    switch (preset)
    {
      case TimingFunctionPreset::Ease:    return Create(TimingFunctionPreset::Ease, 0.25, 0.1, 0.25, 1.0);
      case TimingFunctionPreset::EaseIn:  return Create(TimingFunctionPreset::EaseIn, 0.42, 0.0, 1.0, 1.0);
      case TimingFunctionPreset::EaseOut: return Create(TimingFunctionPreset::EaseOut, 0.0, 0.0, 0.58, 1.0);
      case TimingFunctionPreset::EaseInOut:
        return Create(TimingFunctionPreset::EaseInOut, 0.42, 0.0, 0.58, 1.0);
      case TimingFunctionPreset::Custom: break;
    }
    krys_debug_assert_msg(false, "Unexpected preset");
    return Create();
  }

  bool CubicBezierTimingFunction::Equals(const TimingFunction &other) const noexcept
  {
    const auto *otherCubic = DynamicDowncast<CubicBezierTimingFunction>(other);
    if (!otherCubic || _timingFunctionPreset != otherCubic->_timingFunctionPreset)
    {
      return false;
    }
    if (_timingFunctionPreset != TimingFunctionPreset::Custom)
    {
      return true;
    }
    return _x1 == otherCubic->_x1 && _y1 == otherCubic->_y1 && _x2 == otherCubic->_x2
           && _y2 == otherCubic->_y2;
  }

  bool StepsTimingFunction::Equals(const TimingFunction &other) const noexcept
  {
    const auto *otherSteps = DynamicDowncast<StepsTimingFunction>(other);
    if (!otherSteps || _steps != otherSteps->_steps)
    {
      return false;
    }

    if (_stepPosition == otherSteps->_stepPosition)
    {
      return true;
    }

    if (!_stepPosition && *otherSteps->_stepPosition == StepPosition::End)
    {
      return true;
    }

    if (!otherSteps->_stepPosition && *_stepPosition == StepPosition::End)
    {
      return true;
    }

    return false;
  }

  bool SpringTimingFunction::Equals(const TimingFunction &other) const noexcept
  {
    const auto *otherSpring = DynamicDowncast<SpringTimingFunction>(other);
    return otherSpring && _mass == otherSpring->_mass && _stiffness == otherSpring->_stiffness
           && _damping == otherSpring->_damping && _initialVelocity == otherSpring->_initialVelocity;
  }

  TextStream &operator<<(TextStream &ts, const TimingFunction &timingFunction) noexcept
  {
    switch (timingFunction.GetType())
    {
      case TimingFunction::Type::LinearFunction:
      {
        const auto &function = Downcast<LinearTimingFunction>(timingFunction);
        ts << "linear(";
        for (size_t i = 0; i < function.Points().size(); ++i)
        {
          if (i)
          {
            ts << ", ";
          }

          const auto &point = function.Points()[i];
          ts << point.Value << ' ' << FormatCSSNumber(point.Progress * 100.0) << '%';
        }
        ts << ')';
        break;
      }
      case TimingFunction::Type::CubicBezierFunction:
      {
        const auto &function = Downcast<CubicBezierTimingFunction>(timingFunction);
        ts << "cubic-bezier(" << FormatCSSNumber(function.X1()) << ", " << FormatCSSNumber(function.Y1())
           << ", " << FormatCSSNumber(function.X2()) << ", " << FormatCSSNumber(function.Y2()) << ')';
        break;
      }
      case TimingFunction::Type::StepsFunction:
      {
        const auto &function = Downcast<StepsTimingFunction>(timingFunction);
        ts << "steps(" << static_cast<int32>(function.NumberOfSteps());
        if (auto stepPosition = function.GetStepPosition())
        {
          ts << ", ";
          switch (stepPosition.value())
          {
            case StepsTimingFunction::StepPosition::JumpStart: ts << "jump-start"; break;
            case StepsTimingFunction::StepPosition::JumpEnd:   ts << "jump-end"; break;
            case StepsTimingFunction::StepPosition::JumpNone:  ts << "jump-none"; break;
            case StepsTimingFunction::StepPosition::JumpBoth:  ts << "jump-both"; break;
            case StepsTimingFunction::StepPosition::Start:     ts << "start"; break;
            case StepsTimingFunction::StepPosition::End:       ts << "end"; break;
          }
        }
        ts << ')';
        break;
      }
      case TimingFunction::Type::SpringFunction:
      {
        const auto &function = Downcast<SpringTimingFunction>(timingFunction);
        ts << "spring(" << FormatCSSNumber(function.Mass()) << ' ' << FormatCSSNumber(function.Stiffness())
           << ' ' << FormatCSSNumber(function.Damping()) << ' ' << FormatCSSNumber(function.InitialVelocity())
           << ')';
        break;
      }
    }
    return ts;
  }
}
