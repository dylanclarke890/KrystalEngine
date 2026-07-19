#pragma once

#include "Krystal.HTML/CSS/CSSRule.hpp"

namespace Krys::HTML
{
  class CSSMediaRule : public CSSRule
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(CSSMediaRule);

  public:
    CSSMediaRule(RawPtr<CSSStyleSheet> stylesheet) noexcept;

    ~CSSMediaRule() noexcept override = default;
  };
}