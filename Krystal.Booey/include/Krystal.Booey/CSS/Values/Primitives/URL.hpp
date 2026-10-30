#pragma once

#include "Krystal.Booey/CSS/Values/Primitives/URLModifiers.hpp"
#include "Krystal.Booey/CSS/Values/Types.hpp"
#include "Krystal.Booey/URL/URL.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"

namespace krys::boo::css
{
  struct ParserContext;

  /// @brief A `<url>`: the URL as written, the URL resolved against the base URL, and its modifiers.
  // https://drafts.csswg.org/css-values-4/#url-value
  struct CSSURL
  {
    CSSOMString Specified;
    url::URL Resolved;
    URLModifiers Modifiers;

    /// @brief The absence of a URL (e.g. for `none`).
    KRYS_NODISCARD static CSSURL None() noexcept
    {
      return {.Specified = {}, .Resolved = {}, .Modifiers = {}};
    }

    KRYS_NODISCARD bool IsNone() const noexcept
    {
      // `url("")` has an empty specified URL too, but always a non-null resolved URL.
      return Specified.empty() && Resolved.IsNull();
    }

    KRYS_NODISCARD bool operator==(const CSSURL &) const noexcept = default;
  };

  template <size_t I>
  KRYS_NODISCARD const auto &get(const CSSURL &value) noexcept
  {
    if constexpr (!I)
    {
      return value.Specified;
    }
    if constexpr (I == 1)
    {
      return value.Resolved;
    }
    if constexpr (I == 2)
    {
      return value.Modifiers;
    }
  }

  template <>
  struct Serialiser<CSSURL>
  {
    void operator()(StringBuilder &builder, const SerialisationContext &context,
                    const CSSURL &value) noexcept;
  };

  /// @brief Resolves `string` against the context's base URL. Empty and fragment-only URLs aren't resolved
  /// against the base URL, and nothing is resolved against a null or `about:blank` base URL.
  KRYS_NODISCARD Maybe<CSSURL> CompleteURL(const CSSOMString &string, const ParserContext &context) noexcept;

  /// @brief Parses the specified URL as an absolute URL if it wasn't resolved against a base URL.
  KRYS_NODISCARD CSSURL Resolve(CSSURL &&url) noexcept;

  KRYS_NODISCARD bool MayDependOnBaseURL(const CSSURL &url) noexcept;
}

DEFINE_TUPLE_LIKE_CONFORMANCE(krys::boo::css::CSSURL, 3)
