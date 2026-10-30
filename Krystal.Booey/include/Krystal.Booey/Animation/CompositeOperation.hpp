#pragma once

#include "Krystal.Booey/Graphics/Animation/CompositeOperation.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/Maybe.hpp"

namespace krys::boo
{
  namespace css
  {
    class Value;
  }

  /// @brief The composite operation an `animation-composition` keyword names, if @p value is one.
  KRYS_NODISCARD Maybe<CompositeOperation> ToCompositeOperation(const css::Value &value) noexcept;
}
