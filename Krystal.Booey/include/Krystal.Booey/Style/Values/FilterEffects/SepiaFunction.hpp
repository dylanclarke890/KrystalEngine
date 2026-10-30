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
    struct Sepia;
  }

  namespace style
  {
    class BuilderState;

    KRYS_NODISCARD css::Sepia ToCSSSepia(const BasicColorMatrixFilterOperation &,
                                         const RenderStyle &) noexcept;
    KRYS_NODISCARD Ref<FilterOperation> CreateFilterOperation(const css::Sepia &,
                                                              const BuilderState &) noexcept;
  }
}
