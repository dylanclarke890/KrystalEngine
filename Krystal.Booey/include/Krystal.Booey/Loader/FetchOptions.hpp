#pragma once

#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/Types/String.hpp"

namespace krys::boo
{
  /// @see https://w3c.github.io/webappsec-referrer-policy/#referrer-policies
  enum class ReferrerPolicy : uint8
  {
    EmptyString,
    NoReferrer,
    NoReferrerWhenDowngrade,
    SameOrigin,
    Origin,
    StrictOrigin,
    OriginWhenCrossOrigin,
    StrictOriginWhenCrossOrigin,
    UnsafeUrl,
    Default = StrictOriginWhenCrossOrigin
  };

  /// @brief A fetch's options, as the Fetch specification's request has them: only those that apply without a
  /// network (no mode, credentials, cache or redirect modes).
  struct FetchOptions
  {
    ReferrerPolicy RequestReferrerPolicy {ReferrerPolicy::EmptyString};
    utf8_string Integrity;
  };
}
