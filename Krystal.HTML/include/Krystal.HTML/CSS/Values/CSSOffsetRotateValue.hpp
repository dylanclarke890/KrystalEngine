#pragma once

#include "Krystal.HTML/CSS/Values/CSSValue.hpp"
#include "Krystal.Lib/Pointers/RefPtr.hpp"

namespace Krys::HTML
{
  class CSSOffsetRotateValue final : public CSSValue
  {
  public:
    KRYS_NODISCARD static Ref<CSSOffsetRotateValue> Create(RefPtr<CSSValue> modifier,
                                                           RefPtr<CSSValue> angle) noexcept;
  };
}