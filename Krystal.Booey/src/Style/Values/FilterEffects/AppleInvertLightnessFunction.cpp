#include "Krystal.Booey/Style/Values/FilterEffects/AppleInvertLightnessFunction.hpp"

#include "Krystal.Booey/CSS/Values/FilterEffects/AppleInvertLightnessFunction.hpp"
#include "Krystal.Booey/Graphics/Filters/FilterOperation.hpp"

namespace krys::boo::style
{
  KRYS_NODISCARD css::AppleInvertLightness ToCSSAppleInvertLightness(const InvertLightnessFilterOperation &,
                                                                     const RenderStyle &) noexcept
  {
    return {};
  }

  KRYS_NODISCARD Ref<FilterOperation> CreateFilterOperation(const css::AppleInvertLightness &,
                                                            const BuilderState &) noexcept
  {
    return InvertLightnessFilterOperation::Create();
  }
}
