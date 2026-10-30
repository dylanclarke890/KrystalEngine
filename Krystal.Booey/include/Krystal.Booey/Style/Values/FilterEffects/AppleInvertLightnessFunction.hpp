#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo
{
  class FilterOperation;
  class InvertLightnessFilterOperation;
  class RenderStyle;

  namespace css
  {
    struct AppleInvertLightness;
  }

  namespace style
  {
    class BuilderState;

    KRYS_NODISCARD css::AppleInvertLightness ToCSSAppleInvertLightness(const InvertLightnessFilterOperation &,
                                                                       const RenderStyle &) noexcept;
    KRYS_NODISCARD Ref<FilterOperation> CreateFilterOperation(const css::AppleInvertLightness &,
                                                              const BuilderState &) noexcept;
  }
}
