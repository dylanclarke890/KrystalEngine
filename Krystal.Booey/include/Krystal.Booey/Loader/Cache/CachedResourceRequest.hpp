#pragma once

#include "Krystal.Booey/Loader/ResourceLoaderOptions.hpp"
#include "Krystal.Booey/Platform/Network/ResourceRequest.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/String.hpp"

namespace krys::boo
{
  /// @brief The initiator types a load can report.
  struct CachedResourceRequestInitiatorTypes
  {
    static constexpr utf8_stringview CSS = u8"css";
    static constexpr utf8_stringview Icon = u8"icon";
    static constexpr utf8_stringview XMLHTTPRequest = u8"xmlhttprequest";
  };

  /// @brief A request for a cached resource: the resource request, how to load it and who asked.
  // FIXME: Priorities, preloads, the origin, charsets and access control updates, once there are network
  // loads.
  class CachedResourceRequest
  {
  private:
    ResourceRequest _resourceRequest;
    ResourceLoaderOptions _options;
    utf8_string _initiatorType;
    bool _ignoreForRequestCount {false};

  public:
    explicit CachedResourceRequest(ResourceRequest &&request,
                                   const ResourceLoaderOptions &options = {}) noexcept
        : _resourceRequest(krys::move(request)), _options(options)
    {
    }

    KRYS_NODISCARD const ResourceRequest &GetResourceRequest() const noexcept
    {
      return _resourceRequest;
    }
    KRYS_NODISCARD ResourceRequest &GetResourceRequest() noexcept
    {
      return _resourceRequest;
    }
    KRYS_NODISCARD ResourceRequest &&ReleaseResourceRequest() noexcept
    {
      return krys::move(_resourceRequest);
    }

    KRYS_NODISCARD const ResourceLoaderOptions &Options() const noexcept
    {
      return _options;
    }
    KRYS_NODISCARD bool AllowsCaching() const noexcept
    {
      return _options.Caching == CachingPolicy::AllowCaching
             || _options.Caching == CachingPolicy::AllowCachingMainResourcePrefetch;
    }

    KRYS_NODISCARD const utf8_string &InitiatorType() const noexcept
    {
      return _initiatorType;
    }
    void SetInitiatorType(utf8_stringview initiatorType) noexcept
    {
      _initiatorType = utf8_string(initiatorType);
    }

    KRYS_NODISCARD bool IgnoreForRequestCount() const noexcept
    {
      return _ignoreForRequestCount;
    }
    void SetIgnoreForRequestCount(bool ignoreForRequestCount) noexcept
    {
      _ignoreForRequestCount = ignoreForRequestCount;
    }
  };
}
