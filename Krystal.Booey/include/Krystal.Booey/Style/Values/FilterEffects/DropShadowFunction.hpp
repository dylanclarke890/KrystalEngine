#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo
{
  class FilterOperation;
  class RenderStyle;

  namespace css
  {
    struct DropShadow;
  }

  namespace style
  {
    class BuilderState;
    class DropShadowFilterOperationWithStyleColor;

    KRYS_NODISCARD css::DropShadow ToCSSDropShadow(const DropShadowFilterOperationWithStyleColor &,
                                                   const RenderStyle &) noexcept;
    KRYS_NODISCARD Ref<FilterOperation> CreateFilterOperation(const css::DropShadow &,
                                                              const BuilderState &) noexcept;
  }
}
