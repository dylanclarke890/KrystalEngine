#pragma once

#include "Krystal.HTML/CSS/Values/Primitives/CSSURLModifiers.hpp"
#include "Krystal.HTML/URL/Url.hpp"

namespace Krys::HTML
{
  class Document;

  // https://drafts.csswg.org/css-values-4/#url-value
  struct CSSURL
  {
    CSSOMString specified;
    Url resolved;
    URLModifiers modifiers;

    KRYS_NODISCARD static CSSURL None() noexcept
    {
      return {.specified = {}, .resolved = {}, .modifiers = {}};
    }

    KRYS_NODISCARD bool IsNone() const noexcept
    {
      return specified.empty();
    }

    bool operator==(const CSSURL &) const = default;
  };

  template <size_t I>
  KRYS_NODISCARD const auto &get(const CSSURL &value) noexcept
  {
    if constexpr (!I)
    {
      return value.specified;
    }
    if constexpr (I == 1)
    {
      return value.resolved;
    }
    if constexpr (I == 2)
    {
      return value.modifiers;
    }
  }

  KRYS_NODISCARD Maybe<CSSURL> CompleteURL(const CSSOMString &, const CSSParserContext &) noexcept;
  KRYS_NODISCARD Maybe<CSSURL> CompleteURL(const CSSOMString &, const Document &) noexcept;

  KRYS_NODISCARD CSSURL Resolve(CSSURL &&) noexcept;

  KRYS_NODISCARD bool MayDependOnBaseURL(const CSSURL &) noexcept;
}

DEFINE_TUPLE_LIKE_CONFORMANCE(Krys::HTML::CSSURL, 3)
