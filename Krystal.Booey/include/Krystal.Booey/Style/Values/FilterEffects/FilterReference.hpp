#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo
{
  class FilterOperation;
  class RenderStyle;

  namespace css
  {
    struct FilterReference;
  }

  namespace style
  {
    class BuilderState;
    class ReferenceFilterOperation;

    KRYS_NODISCARD css::FilterReference ToCSSFilterReference(const ReferenceFilterOperation &,
                                                             const RenderStyle &) noexcept;
    KRYS_NODISCARD Ref<FilterOperation> CreateFilterOperation(const css::FilterReference &,
                                                              const BuilderState &) noexcept;
  }
}
