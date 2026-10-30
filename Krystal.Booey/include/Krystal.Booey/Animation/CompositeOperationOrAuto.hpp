#pragma once

#include "Krystal.Booey/Graphics/Animation/CompositeOperation.hpp"
#include "Krystal.Core/Assert.hpp"
#include "Krystal.Core/Attributes.hpp"

namespace krys::boo
{
  /// @brief A keyframe's composite operation, or `auto` for the effect's.
  /// @see https://drafts.csswg.org/web-animations-1/#enumdef-compositeoperationorauto
  enum class CompositeOperationOrAuto : uint8
  {
    Replace,
    Add,
    Accumulate,
    Auto
  };

  KRYS_NODISCARD constexpr CompositeOperationOrAuto
    ToCompositeOperationOrAuto(CompositeOperation operation) noexcept
  {
    switch (operation)
    {
      case CompositeOperation::Replace:    return CompositeOperationOrAuto::Replace;
      case CompositeOperation::Add:        return CompositeOperationOrAuto::Add;
      case CompositeOperation::Accumulate: return CompositeOperationOrAuto::Accumulate;
    }
    krys_debug_assert(false);
    return CompositeOperationOrAuto::Auto;
  }
}
