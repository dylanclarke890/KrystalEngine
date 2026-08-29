#pragma once

#include "Krystal.HTML/CSS/Values/CSSValue.hpp"
#include "Krystal.HTML/CSS/Values/Primitives/CSSURL.hpp"
#include "Krystal.Lib/Pointers/RefPtr.hpp"

namespace Krys::HTML
{
  class CSSURLValue final : public CSSValue
  {
  public:
    KRYS_NODISCARD static Ref<CSSURLValue> Create(const CSSURL &url) noexcept;
  };
}