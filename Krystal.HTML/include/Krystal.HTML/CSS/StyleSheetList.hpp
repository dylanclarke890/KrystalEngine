#pragma once

#include "Krystal.Lib/Pointers/RefPtr.hpp"

namespace Krys::HTML
{
  class CSSStyleSheet;

  /// @see https://www.w3.org/TR/cssom-1/#stylesheetlist
  class StyleSheetList
  {
  public:
#pragma region StyleSheetList - https://www.w3.org/TR/cssom-1/#stylesheetlist

    /// @see https://www.w3.org/TR/cssom-1/#dom-stylesheetlist-item
    KRYS_NODISCARD RefPtr<CSSStyleSheet> Item(size_t index) noexcept;

    /// @see https://www.w3.org/TR/cssom-1/#dom-stylesheetlist-item
    KRYS_NODISCARD RefPtr<const CSSStyleSheet> Item(size_t index) const noexcept;

    /// @see https://www.w3.org/TR/cssom-1/#dom-stylesheetlist-length
    KRYS_NODISCARD size_t Length() const noexcept;

#pragma endregion
  };
}