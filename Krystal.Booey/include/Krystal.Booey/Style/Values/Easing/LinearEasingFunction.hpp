#pragma once

#include "Krystal.Booey/CSS/Values/Easing/LinearEasingFunction.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo
{
  class LinearTimingFunction;
  class RenderStyle;
  class TimingFunction;

  namespace style
  {
    class BuilderState;

    KRYS_NODISCARD css::LinearEasingFunction ToCSSLinearEasingFunction(const LinearTimingFunction &,
                                                                       const RenderStyle &) noexcept;

    KRYS_NODISCARD Ref<TimingFunction> CreateTimingFunction(const BuilderState &,
                                                            const css::LinearEasingFunction &) noexcept;
    KRYS_NODISCARD Ref<TimingFunction>
      CreateTimingFunctionDeprecated(const css::LinearEasingFunction &) noexcept;
  }
}
