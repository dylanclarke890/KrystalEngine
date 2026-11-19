#include "Krystal.Core/Maths/Easing.hpp"
#include <algorithm>
#include <cmath>

namespace krys
{
  double StepEasing(double progress, int32 steps, StepPosition position, bool before) noexcept
  {
    // https://drafts.csswg.org/css-easing-1/#step-easing-algo
    double jumps = steps;

    // 1. Calculate the current step as floor(input progress value × steps).
    auto currentStep = std::floor(progress * jumps);

    // 2. If the step position property is start, increment current step by one.
    if (position == StepPosition::JumpStart || position == StepPosition::Start
        || position == StepPosition::JumpBoth)
    {
      ++currentStep;
    }

    // 3. If both of the following conditions are true:
    //    - the before flag is set, and
    //    - input progress value × steps mod 1 equals zero (that is, if input progress value × steps is
    //      integral), then
    //    decrement current step by one.
    if (before && !std::fmod(progress * jumps, 1))
    {
      --currentStep;
    }

    // 4. If input progress value ≥ 0 and current step < 0, let current step be zero.
    if (progress >= 0 && currentStep < 0)
    {
      currentStep = 0;
    }

    // 5. Calculate jumps based on the step position.
    if (position == StepPosition::JumpNone)
    {
      --jumps;
    }
    else if (position == StepPosition::JumpBoth)
    {
      ++jumps;
    }

    // 6. If input progress value ≤ 1 and current step > jumps, let current step be jumps.
    if (progress <= 1 && currentStep > jumps)
    {
      currentStep = jumps;
    }

    // 7. The output progress value is current step / jumps.
    return currentStep / jumps;
  }

  double LinearEasing(Span<const LinearEasingPoint> points, double progress) noexcept
  {
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
