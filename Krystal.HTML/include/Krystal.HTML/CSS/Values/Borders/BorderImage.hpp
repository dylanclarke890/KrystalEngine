#pragma once

#include "Krystal.Lib/Pointers/RefPtr.hpp"

namespace Krys::HTML
{
  class CSSValue;
  class CSSValueList;

  struct BorderImageComponents
  {
    RefPtr<CSSValue> Source;
    RefPtr<CSSValue> Slice;
    RefPtr<CSSValue> Width;
    RefPtr<CSSValue> Outset;
    RefPtr<CSSValue> Repeat;
  };

  KRYS_NODISCARD Ref<CSSValueList> CreateBorderImageValue(BorderImageComponents &&components) noexcept;
}