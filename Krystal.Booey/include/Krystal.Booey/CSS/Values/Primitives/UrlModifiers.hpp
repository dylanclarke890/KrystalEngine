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
  using UrlCrossOriginParameters = Variant<keywords::Anonymous, keywords::UseCredentials>;
  using UrlCrossOriginFunction = FunctionNotation<ValueId::CrossOrigin, UrlCrossOriginParameters>;

  // <integrity-modifier> = integrity( <string> )
  // https://drafts.csswg.org/css-values-5/#typedef-request-url-modifier-integrity-modifier
  using UrlIntegrityParameters = CSSOMString;
  using UrlIntegrityFunction = FunctionNotation<ValueId::Integrity, UrlIntegrityParameters>;

  // <referrer-policy-modifier> = referrer-policy( no-referrer | no-referrer-when-downgrade | same-origin |
  // origin | strict-origin | origin-when-cross-origin | strict-origin-when-cross-origin | unsafe-url )
  // https://drafts.csswg.org/css-values-5/#typedef-request-url-modifier-referrer-policy-modifier
  using UrlReferrerPolicyParameters =
    Variant<keywords::NoReferrer, keywords::NoReferrerWhenDowngrade, keywords::SameOrigin, keywords::Origin,
            keywords::StrictOrigin, keywords::OriginWhenCrossOrigin, keywords::StrictOriginWhenCrossOrigin,
            keywords::UnsafeUrl>;
  using UrlReferrerPolicyFunction = FunctionNotation<ValueId::ReferrerPolicy, UrlReferrerPolicyParameters>;

  /// @brief The request modifiers of a `<url>`.
  // https://drafts.csswg.org/css-values-5/#typedef-request-url-modifier
  // <request-url-modifier> = <cross-origin-modifier> | <integrity-modifier> | <referrer-policy-modifier>
  struct UrlModifiers
  {
    Maybe<UrlCrossOriginFunction> CrossOrigin;
    Maybe<UrlIntegrityFunction> Integrity;
    Maybe<UrlReferrerPolicyFunction> ReferrerPolicy;

    KRYS_NODISCARD bool operator==(const UrlModifiers &) const noexcept = default;
  };

  template <size_t I>
  KRYS_NODISCARD const auto &get(const UrlModifiers &value) noexcept
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
  void ApplyModifiersToLoaderOptions(const UrlModifiers &modifiers,
                                     ResourceLoaderOptions &loaderOptions) noexcept;
}

KRYS_DEFINE_SPACE_SEPARATED_TUPLE_LIKE_CONFORMANCE(krys::boo::css::UrlModifiers, 3uz)
