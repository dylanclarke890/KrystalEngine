#pragma once

#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::css
{
  class Value;
  class ValueList;

  struct BorderImageComponents
  {
    RefPtr<Value> Source;
    RefPtr<Value> Slice;
    RefPtr<Value> Width;
    RefPtr<Value> Outset;
    RefPtr<Value> Repeat;
  };

  KRYS_NODISCARD Ref<ValueList> CreateBorderImageValue(BorderImageComponents &&components) noexcept;
}