#pragma once

#include "Krystal.Booey/CSS/CSSStyleDeclaration.hpp"

namespace krys::boo::css
{
  /// @see https://drafts.csswg.org/cssom/#the-cssstyledeclaration-interface
  class CSSStyleProperties final : public CSSStyleDeclaration
  {
  public:
#pragma region CSSStyleProperties - https://drafts.csswg.org/cssom/#cssstyleproperties

    /// @see https://drafts.csswg.org/cssom/#dom-cssstyleproperties-cssfloat
    KRYS_NODISCARD CSSOMString CssFloat() const noexcept;

#pragma endregion
  };
}