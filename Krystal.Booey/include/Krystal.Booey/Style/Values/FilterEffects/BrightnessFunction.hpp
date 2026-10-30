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
    struct Brightness;
  }

  namespace style
  {
    class BuilderState;

    KRYS_NODISCARD css::Brightness ToCSSBrightness(const BasicComponentTransferFilterOperation &,
                                                   const RenderStyle &) noexcept;
    KRYS_NODISCARD Ref<FilterOperation> CreateFilterOperation(const css::Brightness &,
                                                              const BuilderState &) noexcept;
  }
}
