#pragma once

#include "Krystal.Booey/CSS/Values/Easing/StepsEasingFunction.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo
{
  class RenderStyle;
  class StepsTimingFunction;
  class TimingFunction;

  namespace style
  {
    class BuilderState;

    KRYS_NODISCARD css::StepsEasingFunction ToCSSStepsEasingFunction(const StepsTimingFunction &,
                                                                     const RenderStyle &) noexcept;

    KRYS_NODISCARD Ref<TimingFunction> CreateTimingFunction(const BuilderState &,
                                                            const css::StepsEasingFunction &) noexcept;
    KRYS_NODISCARD Ref<TimingFunction>
      CreateTimingFunctionDeprecated(const css::StepsEasingFunction &) noexcept;
  }
}
