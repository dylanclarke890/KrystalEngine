#pragma once

#include "Krystal.Booey/Loader/ResourceLoaderOptions.hpp"
#include "Krystal.Booey/Platform/Network/ResourceError.hpp"
#include "Krystal.Booey/Platform/Network/ResourceRequest.hpp"
#include "Krystal.Booey/Platform/Network/ResourceResponse.hpp"
#include "Krystal.Booey/Platform/SharedBuffer.hpp"
#include "Krystal.Core/Types/Expected.hpp"
#include "Krystal.Core/Types/Func.hpp"

namespace krys::boo
{
  class LocalFrame;

  /// @brief A resource the host loaded: its response and all of its bytes.
  struct LoadedResource
  {
    ResourceResponse Response;
    Ref<SharedBuffer> Data;
  };

  using LoadResult = Expected<LoadedResource, ResourceError>;

  /// @brief How resources are loaded: the host implements it, Booey never reads files or the network itself.
  /// `data:` URLs are decoded by Booey and never reach the strategy.
  /// FIXME: Streamed responses (data arriving in parts), redirects, cancellation and synchronous loads.
  class LoaderStrategy
  {
  public:
    virtual ~LoaderStrategy() noexcept = default;

    /// @brief Loads @p request's URL for @p frame, then calls @p completionHandler once, with the resource
    /// or the reason it couldn't load.
    /// @note The handler may be called at any time on the main thread, even before this returns: Booey
    /// queues the completion as a networking task either way.
    virtual void LoadResource(LocalFrame &frame, const ResourceRequest &request,
                              const ResourceLoaderOptions &options,
                              Func<void(LoadResult &&)> &&completionHandler) noexcept = 0;

    /// @brief Whether the network is reachable (`navigator.onLine`); the host tells pages when it changes
    /// through `Page::NetworkStateChanged`.
    KRYS_NODISCARD virtual bool IsOnLine() const noexcept
    {
      return true;
    }
  };
}
