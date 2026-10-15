#pragma once

#include "Krystal.Booey/CSS/Values/Easing/SpringEasingFunction.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo
{
  class RenderStyle;
  class SpringTimingFunction;
  class TimingFunction;

  namespace style
  {
    class BuilderState;

    KRYS_NODISCARD css::SpringEasingFunction ToCSSSpringEasingFunction(const SpringTimingFunction &,
                                                                       const RenderStyle &) noexcept;

    KRYS_NODISCARD Ref<TimingFunction> CreateTimingFunction(const BuilderState &,
                                                            const css::SpringEasingFunction &) noexcept;
    KRYS_NODISCARD Ref<TimingFunction>
      CreateTimingFunctionDeprecated(const css::SpringEasingFunction &) noexcept;
  }
}
