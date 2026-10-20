#pragma once

#include "Krystal.Booey/Loader/FetchOptions.hpp"
#include "Krystal.Booey/Loader/LoadedFromOpaqueSource.hpp"

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
  // FIXME: The security, CORS and service worker policies, once there are network loads.
  struct ResourceLoaderOptions : FetchOptions
  {
    SendCallbackPolicy SendLoadCallbacks {SendCallbackPolicy::DoNotSendCallbacks};
    DataBufferingPolicy DataBuffering {DataBufferingPolicy::BufferData};
    CachingPolicy Caching {CachingPolicy::AllowCaching};
    ContentSecurityPolicyImposition ContentSecurityPolicy {ContentSecurityPolicyImposition::DoPolicyCheck};
    LoadedFromOpaqueSource IsLoadedFromOpaqueSource {LoadedFromOpaqueSource::No};
  };
}
