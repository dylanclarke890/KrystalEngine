/*
 * Copyright (C) 2025 Samuel Weinig <sam@webkit.org>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

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
