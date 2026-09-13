#pragma once

#include "Krystal.Booey/CSS/Values/CSSValueTypes.hpp"

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

DEFINE_SPACE_SEPARATED_TUPLE_LIKE_CONFORMANCE(krys::boo::css::URLModifiers, 3uz)
