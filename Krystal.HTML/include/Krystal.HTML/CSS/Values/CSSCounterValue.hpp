#pragma once

#include "Krystal.HTML/CSS/Types/CSSOMString.hpp"
#include "Krystal.HTML/CSS/Values/CSSValue.hpp"
#include "Krystal.Lib/Pointers/RefPtr.hpp"

namespace Krys::HTML
{
  class CSSCounterValue : public CSSValue
  {
  public:
    KRYS_NODISCARD static Ref<CSSCounterValue> Create(CSSOMStringAtom identifier, CSSOMStringAtom separator,
                                                      Ref<CSSValue> counterStyle) noexcept;
  };
}