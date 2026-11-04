#pragma once

#include "Krystal.Booey/Loader/FetchOptions.hpp"

namespace krys::boo
{
  enum class SendCallbackPolicy : bool
  {
    SendCallbacks,
    DoNotSendCallbacks
  };

  enum class DataBufferingPolicy : bool
  {
    BufferData,
    DoNotBufferData
  };

  enum class CachingPolicy : uint8
  {
    AllowCaching,
    DisallowCaching,
    AllowCachingMainResourcePrefetch,
  };

  enum class ContentSecurityPolicyImposition : bool
  {
    SkipPolicyCheck,
    DoPolicyCheck
  };

  /// @brief How a resource is loaded: its fetch options and the loader's policies.
  struct ResourceLoaderOptions : FetchOptions
  {
    SendCallbackPolicy SendLoadCallbacks {SendCallbackPolicy::DoNotSendCallbacks};
    DataBufferingPolicy DataBuffering {DataBufferingPolicy::BufferData};
    CachingPolicy Caching {CachingPolicy::AllowCaching};
    ContentSecurityPolicyImposition ContentSecurityPolicy {ContentSecurityPolicyImposition::DoPolicyCheck};
  };
}
