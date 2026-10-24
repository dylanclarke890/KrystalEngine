#pragma once

#include "Krystal.Booey/DOM/Types/DOMString.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/Maybe.hpp"

namespace krys::boo::html
{
  /// @brief A declarative refresh: after how many seconds, and to which URL (empty for the document's).
  struct RefreshDirective
  {
    double Delay {0};
    dom::DOMString URL;
  };

  /// @brief Parses @p input, a `<meta http-equiv=refresh>`'s content (or a `Refresh` header): seconds, then
  /// optionally a URL, after `;` or `,` and `url=`, perhaps quoted.
  /// @return Null if it doesn't start with a time.
  /// @see https://html.spec.whatwg.org/#shared-declarative-refresh-steps
  KRYS_NODISCARD Maybe<RefreshDirective> ParseRefresh(dom::DOMStringView input) noexcept;
}
