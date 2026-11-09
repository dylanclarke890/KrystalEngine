#pragma once

#include "Krystal.Booey/DOM/Types/DOMString.hpp"
#include "Krystal.Booey/URL/URL.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/Types/Maybe.hpp"

namespace krys::boo
{
  /// @brief An icon a document names with `<link rel=icon>`: where it is, and what its `type` and `sizes`
  /// say about it.
  /// @see https://html.spec.whatwg.org/#rel-icon
  struct LinkIcon
  {
    url::URL URL;
    /// @brief Its `type` attribute, a hint of its MIME type; empty without one.
    dom::DOMString MIMEType;
    /// @brief The width its `sizes` gives first (`60x70` is 60); none without one, or for `any`.
    Maybe<uint32> Size;

    KRYS_NODISCARD bool operator==(const LinkIcon &) const noexcept = default;
  };

  /// @brief An icon whose load the host decides on, with the identifier its decision is given with.
  struct LinkIconLoadDecision
  {
    const LinkIcon &Icon;
    uint64 Identifier;
  };
}
