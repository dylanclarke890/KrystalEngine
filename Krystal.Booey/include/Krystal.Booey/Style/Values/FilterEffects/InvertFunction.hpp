#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo
{
  class BasicComponentTransferFilterOperation;
  class FilterOperation;
  class RenderStyle;

  namespace css
  {
    struct Invert;
  }

  namespace style
  {
    class BuilderState;

    KRYS_NODISCARD css::Invert ToCSSInvert(const BasicComponentTransferFilterOperation &,
                                           const RenderStyle &) noexcept;
    KRYS_NODISCARD Ref<FilterOperation> CreateFilterOperation(const css::Invert &,
                                                              const BuilderState &) noexcept;
  }
}
