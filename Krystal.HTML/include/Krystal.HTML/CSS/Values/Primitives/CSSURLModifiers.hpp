#pragma once

#include "Krystal.HTML/CSS/Values/CSSValueTypes.hpp"

namespace Krys::HTML
{
  // <cross-origin-modifier> = cross-origin( anonymous | use-credentials )
  // https://drafts.csswg.org/css-values-5/#typedef-request-url-modifier-cross-origin-modifier
  using URLCrossOriginParameters = Variant<Keywords::Anonymous, Keywords::UseCredentials>;
  using URLCrossOriginFunction = FunctionNotation<CSSValueId::CrossOrigin, URLCrossOriginParameters>;

  // <integrity-modifier> = integrity( <string> )
  // https://drafts.csswg.org/css-values-5/#typedef-request-url-modifier-integrity-modifier
  using URLIntegrityParameters = CSSOMString;
  using URLIntegrityFunction = FunctionNotation<CSSValueId::Integrity, URLIntegrityParameters>;

  // <referrer-policy-modifier> = referrer-policy( no-referrer | no-referrer-when-downgrade | same-origin |
  // origin | strict-origin | origin-when-cross-origin | strict-origin-when-cross-origin | unsafe-url )
  // https://drafts.csswg.org/css-values-5/#typedef-request-url-modifier-referrer-policy-modifier
  using URLReferrerPolicyParameters =
    Variant<Keywords::NoReferrer, Keywords::NoReferrerWhenDowngrade, Keywords::SameOrigin, Keywords::Origin,
            Keywords::StrictOrigin, Keywords::OriginWhenCrossOrigin, Keywords::StrictOriginWhenCrossOrigin,
            Keywords::UnsafeUrl>;
  using URLReferrerPolicyFunction = FunctionNotation<CSSValueId::ReferrerPolicy, URLReferrerPolicyParameters>;

  // https://drafts.csswg.org/css-values-5/#typedef-request-url-modifier
  // <request-url-modifier> = <cross-origin-modifier> | <integrity-modifier> | <referrer-policy-modifier>
  struct URLModifiers
  {
    Maybe<URLCrossOriginFunction> crossOrigin {};
    Maybe<URLIntegrityFunction> integrity {};
    Maybe<URLReferrerPolicyFunction> referrerPolicy {};

    // This is not a parsed value, but is implicit from context the modifiers were parsed with.
    // LoadedFromOpaqueSource loadedFromOpaqueSource {LoadedFromOpaqueSource::No};

    KRYS_NODISCARD bool operator==(const URLModifiers &) const noexcept = default;
  };

  template <size_t I>
  KRYS_NODISCARD const auto &get(const URLModifiers &value) noexcept
  {
    if constexpr (I == 0)
    {
      return value.crossOrigin;
    }
    else if constexpr (I == 1)
    {
      return value.integrity;
    }
    else if constexpr (I == 2)
    {
      return value.referrerPolicy;
    }
  }
}

DEFINE_SPACE_SEPARATED_TUPLE_LIKE_CONFORMANCE(Krys::HTML::URLModifiers, 3uz)
