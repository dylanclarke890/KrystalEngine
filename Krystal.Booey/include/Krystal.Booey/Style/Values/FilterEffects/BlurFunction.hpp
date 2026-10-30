#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo
{
  class BlurFilterOperation;
  class FilterOperation;
  class RenderStyle;

  namespace css
  {
    struct Blur;
  }

  namespace style
  {
    class BuilderState;

    KRYS_NODISCARD css::Blur ToCSSBlur(const BlurFilterOperation &, const RenderStyle &) noexcept;
    KRYS_NODISCARD Ref<FilterOperation> CreateFilterOperation(const css::Blur &,
                                                              const BuilderState &) noexcept;
  }
}
