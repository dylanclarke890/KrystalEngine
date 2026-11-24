#pragma once

#include "Krystal.Core/Memory/RefPtr.hpp"

namespace krys::boo::css
{
  class Value;
  class ValueList;

  /// @brief The parts of a `border-image` or `mask-border`, each null when it's left out.
  /// @see https://www.w3.org/TR/css-backgrounds-3/#propdef-border-image
  struct BorderImageComponents
  {
    RefPtr<Value> Source;
    RefPtr<Value> Slice;
    RefPtr<Value> Width;
    RefPtr<Value> Outset;
    RefPtr<Value> Repeat;
  };

  /// @brief The parts as one value: `source slice / width / outset repeat`, without the parts left out.
  /// @pre An outset needs a width, since `slice / outset` would read back as a width.
  KRYS_NODISCARD Ref<ValueList> CreateBorderImageValue(BorderImageComponents components) noexcept;
}
