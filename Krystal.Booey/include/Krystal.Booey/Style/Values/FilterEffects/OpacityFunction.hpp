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
    struct Opacity;
  }

  namespace style
  {
    class BuilderState;

    KRYS_NODISCARD css::Opacity ToCSSOpacity(const BasicComponentTransferFilterOperation &,
                                             const RenderStyle &) noexcept;
    KRYS_NODISCARD Ref<FilterOperation> CreateFilterOperation(const css::Opacity &,
                                                              const BuilderState &) noexcept;
  }
}
