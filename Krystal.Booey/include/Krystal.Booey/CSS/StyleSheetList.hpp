#pragma once

#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::css
{
  class CSSStyleSheet;

  /// @see https://drafts.csswg.org/cssom/#the-stylesheetlist-interface
  class StyleSheetList
  {
  public:
#pragma region StyleSheetList - https://drafts.csswg.org/cssom/#stylesheetlist

    /// @see https://drafts.csswg.org/cssom/#dom-stylesheetlist-item
    KRYS_NODISCARD RefPtr<CSSStyleSheet> Item(size_t index) noexcept;

    /// @see https://drafts.csswg.org/cssom/#dom-stylesheetlist-item
    KRYS_NODISCARD RefPtr<const CSSStyleSheet> Item(size_t index) const noexcept;

    /// @see https://drafts.csswg.org/cssom/#dom-stylesheetlist-length
    KRYS_NODISCARD size_t Length() const noexcept;

#pragma endregion
  };
}