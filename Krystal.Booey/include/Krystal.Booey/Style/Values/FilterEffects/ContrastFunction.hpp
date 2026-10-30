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
    struct Contrast;
  }

  namespace style
  {
    class BuilderState;

    KRYS_NODISCARD css::Contrast ToCSSContrast(const BasicComponentTransferFilterOperation &,
                                               const RenderStyle &) noexcept;
    KRYS_NODISCARD Ref<FilterOperation> CreateFilterOperation(const css::Contrast &,
                                                              const BuilderState &) noexcept;
  }
}
