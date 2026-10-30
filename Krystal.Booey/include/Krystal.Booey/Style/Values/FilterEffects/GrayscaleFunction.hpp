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
    struct Grayscale;
  }

  namespace style
  {
    class BuilderState;

    KRYS_NODISCARD css::Grayscale ToCSSGrayscale(const BasicColorMatrixFilterOperation &,
                                                 const RenderStyle &) noexcept;
    KRYS_NODISCARD Ref<FilterOperation> CreateFilterOperation(const css::Grayscale &,
                                                              const BuilderState &) noexcept;
  }
}
