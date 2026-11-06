#pragma once

#include "Krystal.Booey/CSS/Types/CSSOMString.hpp"
#include "Krystal.Core/Attributes.hpp"

namespace krys::boo::css
{
  /// @brief The `CSS` namespace.
  /// @see https://drafts.csswg.org/cssom/#namespacedef-css
  struct DOMCSSNamespace
  {
    /// @brief @p ident escaped for use as a CSS identifier.
    /// @see https://drafts.csswg.org/cssom/#dom-css-escape
    KRYS_NODISCARD static CSSOMString Escape(const CSSOMString &ident) noexcept;
  };
}
