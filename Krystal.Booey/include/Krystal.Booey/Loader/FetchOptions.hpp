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

  /// @brief A fetch's options, as the Fetch specification's request has them.
  // FIXME: The destination, cache mode, redirect mode, keepalive and client identifiers, once there are
  // network loads.
  struct FetchOptions
  {
    enum class Mode : uint8
    {
      Navigate,
      SameOrigin,
      NoCors,
      Cors
    };
    enum class Credentials : uint8
    {
      Omit,
      SameOrigin,
      Include
    };

    Mode RequestMode {Mode::NoCors};
    Credentials CredentialsMode {Credentials::Include};
    ReferrerPolicy RequestReferrerPolicy {ReferrerPolicy::EmptyString};
    utf8_string Integrity;
  };
}
