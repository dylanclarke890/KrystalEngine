#pragma once

#include "Krystal.HTML/CSS/Values/CSSValue.hpp"
#include "Krystal.HTML/CSS/Values/CSSValueListBuilder.hpp"
#include "Krystal.Lib/Pointers/RefPtr.hpp"

namespace Krys::HTML
{
  class CSSFunctionValue final : public CSSValue
  {
  public:
    KRYS_NODISCARD static Ref<CSSFunctionValue> Create(CSSValueId name, CSSValueListBuilder &&args) noexcept;
  };
}