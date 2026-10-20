#pragma once

#include "Krystal.Booey/HTML/EventLoop.hpp"
#include "Krystal.Booey/Loader/Cache/MemoryCache.hpp"
#include "Krystal.Booey/Loader/LoaderStrategy.hpp"
#include "Krystal.Booey/Platform/Network/ResourceError.hpp"
#include "Krystal.Booey/Platform/Network/ResourceRequest.hpp"
#include "Krystal.Booey/Platform/Network/ResourceResponse.hpp"
#include "Krystal.Booey/Platform/PlatformStrategies.hpp"
#include "Krystal.Booey/Platform/SharedBuffer.hpp"
#include "Krystal.Core/Types/HashMap.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/Span.hpp"

namespace krys::boo::tests
{
  /// @brief Serves images from a map by URL, and records what's asked for; can hold completions back.
  class TestLoaderStrategy final : public LoaderStrategy
  {
  public:
    HashMap<utf8_string, Span<const uint8>> Resources;
    List<utf8_string> Requests;
    bool Defer {false};
    List<Pair<url::URL, Func<void(LoadResult &&)>>> Deferred;

    void LoadResource(LocalFrame &, const ResourceRequest &request, const ResourceLoaderOptions &,
                      Func<void(LoadResult &&)> &&completionHandler) noexcept override
    {
      Requests.push_back(request.URL().String());
      if (Defer)
      {
        Deferred.emplace_back(request.URL(), krys::move(completionHandler));
        return;
      }
      Complete(request.URL(), completionHandler);
    }

    void CompleteDeferred() noexcept
    {
      auto deferred = krys::move(Deferred);
      Deferred.clear();
      for (auto &[url, completionHandler] : deferred)
      {
        Complete(url, completionHandler);
      }
    }

  private:
    void Complete(const url::URL &url, const Func<void(LoadResult &&)> &completionHandler) noexcept
    {
      auto found = Resources.find(url.String());
      if (found == Resources.end())
      {
        completionHandler(Unexpected(ResourceError(u8"Test", 404, url, u8"Not found")));
        return;
      }
      completionHandler(LoadedResource {
        .Response = ResourceResponse(url, u8"image/png", static_cast<int64>(found->second.size())),
        .Data = SharedBuffer::Create(found->second)});
    }
  };

  /// @brief Makes a test strategy Booey's loader for a test, with an empty memory cache.
  struct LoaderFixture
  {
    TestLoaderStrategy Strategy;
    LoaderStrategy *Previous {PlatformStrategies::Singleton().GetLoaderStrategy()};

    LoaderFixture() noexcept
    {
      MemoryCache::Singleton().EvictResources();
      PlatformStrategies::Singleton().SetLoaderStrategy(&Strategy);
    }

    ~LoaderFixture() noexcept
    {
      PlatformStrategies::Singleton().SetLoaderStrategy(Previous);
      MemoryCache::Singleton().EvictResources();
    }
  };

  /// @brief Runs the networking tasks that complete loads, and the timers and tasks they lead to.
  inline void RunNetworkingTasks() noexcept
  {
    html::EventLoop::Current().RunUntilIdle();
  }
}
