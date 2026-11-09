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

#include "Krystal.Booey/CSS/Values/Types.hpp"

namespace krys::boo
{
  struct ResourceLoaderOptions;
}

namespace krys::boo::css
{
  // <cross-origin-modifier> = cross-origin( anonymous | use-credentials )
  // https://drafts.csswg.org/css-values-5/#typedef-request-url-modifier-cross-origin-modifier
  using URLCrossOriginParameters = Variant<keywords::Anonymous, keywords::UseCredentials>;
  using URLCrossOriginFunction = FunctionNotation<ValueId::CrossOrigin, URLCrossOriginParameters>;

  // <integrity-modifier> = integrity( <string> )
  // https://drafts.csswg.org/css-values-5/#typedef-request-url-modifier-integrity-modifier
  using URLIntegrityParameters = CSSOMString;
  using URLIntegrityFunction = FunctionNotation<ValueId::Integrity, URLIntegrityParameters>;

  // <referrer-policy-modifier> = referrer-policy( no-referrer | no-referrer-when-downgrade | same-origin |
  // origin | strict-origin | origin-when-cross-origin | strict-origin-when-cross-origin | unsafe-url )
  // https://drafts.csswg.org/css-values-5/#typedef-request-url-modifier-referrer-policy-modifier
  using URLReferrerPolicyParameters =
    Variant<keywords::NoReferrer, keywords::NoReferrerWhenDowngrade, keywords::SameOrigin, keywords::Origin,
            keywords::StrictOrigin, keywords::OriginWhenCrossOrigin, keywords::StrictOriginWhenCrossOrigin,
            keywords::UnsafeUrl>;
  using URLReferrerPolicyFunction = FunctionNotation<ValueId::ReferrerPolicy, URLReferrerPolicyParameters>;

  /// @brief The request modifiers of a `<url>`.
  // https://drafts.csswg.org/css-values-5/#typedef-request-url-modifier
  // <request-url-modifier> = <cross-origin-modifier> | <integrity-modifier> | <referrer-policy-modifier>
  struct URLModifiers
  {
    Maybe<URLCrossOriginFunction> CrossOrigin {};
    Maybe<URLIntegrityFunction> Integrity {};
    Maybe<URLReferrerPolicyFunction> ReferrerPolicy {};

    KRYS_NODISCARD bool operator==(const URLModifiers &) const noexcept = default;
  };

  template <size_t I>
  KRYS_NODISCARD const auto &get(const URLModifiers &value) noexcept
  {
    if constexpr (I == 0)
    {
      return value.CrossOrigin;
    }
    else if constexpr (I == 1)
    {
      return value.Integrity;
    }
    else if constexpr (I == 2)
    {
      return value.ReferrerPolicy;
    }
  }

  /// @brief Applies the URL request modifier steps of @p modifiers to @p loaderOptions.
  void ApplyModifiersToLoaderOptions(const URLModifiers &modifiers,
                                     ResourceLoaderOptions &loaderOptions) noexcept;
}

DEFINE_SPACE_SEPARATED_TUPLE_LIKE_CONFORMANCE(krys::boo::css::URLModifiers, 3uz)
