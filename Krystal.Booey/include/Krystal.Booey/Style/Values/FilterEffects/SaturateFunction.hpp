#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo
{
  class BasicColorMatrixFilterOperation;
  class FilterOperation;
  class RenderStyle;

  namespace css
  {
    struct Saturate;
  }

  namespace style
  {
    class BuilderState;

    KRYS_NODISCARD css::Saturate ToCSSSaturate(const BasicColorMatrixFilterOperation &,
                                               const RenderStyle &) noexcept;
    KRYS_NODISCARD Ref<FilterOperation> CreateFilterOperation(const css::Saturate &,
                                                              const BuilderState &) noexcept;
  }
}
