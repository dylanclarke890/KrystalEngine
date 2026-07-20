#pragma once

#include "Krystal.HTML/CSS/CSSStyleDeclaration.hpp"

namespace Krys::HTML
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