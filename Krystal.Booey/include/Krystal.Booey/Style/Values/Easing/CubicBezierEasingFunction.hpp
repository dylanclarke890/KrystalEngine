#pragma once

#include "Krystal.Booey/CSS/Values/Easing/CubicBezierEasingFunction.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo
{
  class CubicBezierTimingFunction;
  class RenderStyle;
  class TimingFunction;

  namespace style
  {
    class BuilderState;

    KRYS_NODISCARD css::CubicBezierEasingFunction
      ToCSSCubicBezierEasingFunction(const CubicBezierTimingFunction &, const RenderStyle &) noexcept;

    KRYS_NODISCARD Ref<TimingFunction> CreateTimingFunction(const BuilderState &,
                                                            const css::CubicBezierEasingFunction &) noexcept;
    KRYS_NODISCARD Ref<TimingFunction>
      CreateTimingFunctionDeprecated(const css::CubicBezierEasingFunction &) noexcept;
  }
}
