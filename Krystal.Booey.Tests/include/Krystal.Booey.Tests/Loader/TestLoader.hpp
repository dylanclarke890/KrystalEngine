#pragma once

#include "catch2/catch_test_macros.hpp"
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
#include "Krystal.Core/Types/UniquePtr.hpp"
#include <filesystem>
#include <fstream>
#include <iterator>

namespace krys::boo::tests
{
  /// @brief Serves resources from a map by URL (images, unless given another type), and records what's asked
  /// for; can hold completions back.
  class TestLoaderStrategy final : public LoaderStrategy
  {
  private:
    List<UniquePtr<utf8_string>> _texts;
    List<UniquePtr<List<uint8>>> _files;

  public:
    HashMap<utf8_string, Span<const uint8>> Resources;
    HashMap<utf8_string, utf8_string> MimeTypes;
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

    /// @brief Serves @p text at @p url as @p mimeType.
    void AddText(utf8_stringview url, utf8_stringview text, utf8_stringview mimeType = u8"text/css") noexcept
    {
      auto &stored = *_texts.emplace_back(CreateUnique<utf8_string>(text));
      Resources[utf8_string(url)] =
        Span<const uint8>(reinterpret_cast<const uint8 *>(stored.data()), stored.size());
      MimeTypes[utf8_string(url)] = utf8_string(mimeType);
    }

    /// @brief Serves the bytes of the file at @p path at @p url as @p mimeType.
    void AddFile(utf8_stringview url, const std::filesystem::path &path, utf8_stringview mimeType) noexcept
    {
      std::ifstream stream(path, std::ios::binary);
      REQUIRE(stream);
      auto &stored = *_files.emplace_back(
        CreateUnique<List<uint8>>(std::istreambuf_iterator<char>(stream), std::istreambuf_iterator<char>()));
      Resources[utf8_string(url)] = Span<const uint8>(stored.data(), stored.size());
      MimeTypes[utf8_string(url)] = utf8_string(mimeType);
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
      auto mimeType = MimeTypes.find(url.String());
      completionHandler(LoadedResource {
        .Response =
          ResourceResponse(url, mimeType != MimeTypes.end() ? mimeType->second : utf8_string(u8"image/png"),
                           static_cast<int64>(found->second.size())),
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
