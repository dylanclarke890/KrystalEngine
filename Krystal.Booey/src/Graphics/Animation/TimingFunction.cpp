#include "Krystal.Booey/Graphics/Animation/TimingFunction.hpp"
#include "Krystal.Booey/Core/NumberFormatting.hpp"
#include "Krystal.Booey/Graphics/SpringSolver.hpp"
#include "Krystal.Booey/Graphics/UnitBezier.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"
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
        // https://drafts.csswg.org/css-easing-1/#step-timing-functions
        const auto &function = Downcast<StepsTimingFunction>(*this);
        double steps = function.NumberOfSteps();
        auto stepPosition = function.GetStepPosition();
        // 1. Calculate the current step as floor(input progress value × steps).
        auto currentStep = std::floor(progress * steps);
        // 2. If the step position property is start, increment current step by one.
        if (stepPosition == StepsTimingFunction::StepPosition::JumpStart
            || stepPosition == StepsTimingFunction::StepPosition::Start
            || stepPosition == StepsTimingFunction::StepPosition::JumpBoth)
        {
          ++currentStep;
        }
        // 3. If both of the following conditions are true:
        //    - the before flag is set, and
        //    - input progress value × steps mod 1 equals zero (that is, if input progress value × steps is
        //      integral), then
        //    decrement current step by one.
        if (before == Before::Yes && !std::fmod(progress * steps, 1))
        {
          currentStep--;
        }
        // 4. If input progress value ≥ 0 and current step < 0, let current step be zero.
        if (progress >= 0 && currentStep < 0)
        {
          currentStep = 0;
        }
        // 5. Calculate jumps based on the step position.
        if (stepPosition == StepsTimingFunction::StepPosition::JumpNone)
        {
          --steps;
        }
        else if (stepPosition == StepsTimingFunction::StepPosition::JumpBoth)
        {
          ++steps;
        }
        // 6. If input progress value ≤ 1 and current step > jumps, let current step be jumps.
        if (progress <= 1 && currentStep > steps)
        {
          currentStep = steps;
        }
        // 7. The output progress value is current step / jumps.
        return currentStep / steps;
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
        const auto &function = Downcast<LinearTimingFunction>(*this);

        const auto &points = function.Points();
        if (points.size() < 2)
        {
          return progress;
        }

        // The last point whose progress is at or before `progress` (the first point if there's none), so that
        // `i + 1` is a point.
        auto last = std::find_if(points.rbegin(), points.rend(),
                                 [&](const auto &point) { return point.Progress <= progress; });
        size_t i = last == points.rend() ? 0 : static_cast<size_t>(points.rend() - last) - 1;
        if (i == points.size() - 1)
        {
          --i;
        }

        if (points[i].Progress == points[i + 1].Progress)
        {
          return points[i + 1].Value;
        }

        return points[i].Value
               + ((progress - points[i].Progress) / (points[i + 1].Progress - points[i].Progress)
                  * (points[i + 1].Value - points[i].Value));
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
}
