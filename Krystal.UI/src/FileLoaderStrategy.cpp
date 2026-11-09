#include "Krystal.UI/FileLoaderStrategy.hpp"
#include "Krystal.Booey/HTML/EventLoop/EventLoop.hpp"
#include "Krystal.Booey/Platform/MIMETypeRegistry.hpp"
#include "Krystal.Core/Text/ASCII.hpp"
#include "Krystal.Core/Threading/WorkQueue.hpp"
#include <algorithm>
#include <chrono>
#include <filesystem>
#include <format>
#include <fstream>
#include <iterator>
#include <string>

namespace krys::ui
{
  namespace
  {
    constexpr utf8_stringview ErrorDomain = u8"KrystalUI";

    /// @brief A file's bytes, and its modification time as an HTTP-date.
    struct FileContents
    {
      List<uint8> Bytes;
      Maybe<std::string> LastModified;
    };

    /// @brief Reads the file at @p path, or nothing if it can't be opened. It touches nothing of Booey's, so
    /// it can run on any thread.
    KRYS_NODISCARD Maybe<FileContents> ReadFile(const std::filesystem::path &path) noexcept
    {
      std::ifstream file(path, std::ios::binary);
      if (!file)
      {
        return null;
      }

      FileContents contents {
        .Bytes = {std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>()}};
      // The file's modification time is its document's lastModified.
      std::error_code error;
      if (auto writeTime = std::filesystem::last_write_time(path, error); !error)
      {
        auto modified = std::chrono::floor<std::chrono::seconds>(
          std::chrono::clock_cast<std::chrono::system_clock>(writeTime));
        contents.LastModified = std::format("{:%a, %d %b %Y %H:%M:%S} GMT", modified);
      }
      return contents;
    }

    /// @brief What loading @p url, the file at @p path, gave: @p contents as a resource, or an error if the
    /// file couldn't be read.
    KRYS_NODISCARD boo::LoadResult LoadResultFor(const boo::url::URL &url, const std::filesystem::path &path,
                                                 Maybe<FileContents> &&contents) noexcept
    {
      if (!contents)
      {
        return Unexpected(boo::ResourceError(utf8_string(ErrorDomain), 0, url, u8"Couldn't open the file"));
      }

      // The type is the extension's, without its dot.
      const auto extension = path.extension().u8string();
      const auto size = static_cast<int64>(contents->Bytes.size());
      boo::ResourceResponse response(url,
                                     boo::MIMETypeRegistry::MIMETypeForExtension(
                                       utf8_stringview(extension).substr(extension.empty() ? 0 : 1)),
                                     size);
      if (const auto &date = contents->LastModified)
      {
        response.SetHTTPHeaderField(
          u8"Last-Modified", utf8_stringview(reinterpret_cast<const char8_t *>(date->data()), date->size()));
      }
      return boo::LoadedResource {.Response = krys::move(response),
                                  .Data = boo::SharedBuffer::Create(krys::move(contents->Bytes))};
    }
  }

  void FileLoaderStrategy::LoadResource(boo::LocalFrame &, const boo::ResourceRequest &request,
                                        const boo::ResourceLoaderOptions &,
                                        Func<void(boo::LoadResult &&)> &&completionHandler) noexcept
  {
    const auto &url = request.URL();
    if (!url.ProtocolIsFile())
    {
      completionHandler(
        Unexpected(boo::ResourceError(utf8_string(ErrorDomain), 0, url, u8"Only file URLs load")));
      return;
    }

    std::filesystem::path path(url.FileSystemPath());
    if (!_readQueue)
    {
      completionHandler(LoadResultFor(url, path, ReadFile(path)));
      return;
    }

    // Only the read leaves the main thread: the response is made from it back on the main thread, as
    // Booey's types are the main thread's.
    _readQueue->Dispatch([path] { return ReadFile(path); }, boo::html::EventLoop::Current().Inbox(),
                         [url = boo::url::URL(url), path, completionHandler = krys::move(completionHandler)](
                           Maybe<FileContents> contents) mutable
                         { completionHandler(LoadResultFor(url, path, krys::move(contents))); },
                         {.MayBlock = true});
  }
}
