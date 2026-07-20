#pragma once

#include "Krystal.HTML/CSS/CSSStyleDeclaration.hpp"

namespace Krys::HTML
{
  /// @see https://www.w3.org/TR/cssom-1/#cssstyleproperties
  class CSSStyleProperties final : public CSSStyleDeclaration
  {
  public:
#pragma region CSSStyleProperties - https://www.w3.org/TR/cssom-1/#cssstyleproperties

    /// @see https://www.w3.org/TR/cssom-1/#dom-cssstyledeclaration-cssfloat
    KRYS_NODISCARD CSSOMString CssFloat() const noexcept;

#pragma endregion
  };
}