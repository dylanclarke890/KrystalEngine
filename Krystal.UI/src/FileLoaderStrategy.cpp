#include "Krystal.UI/FileLoaderStrategy.hpp"
#include "Krystal.Booey/Platform/MIMETypeRegistry.hpp"
#include "Krystal.Core/Text/ASCII.hpp"
#include <algorithm>
#include <chrono>
#include <filesystem>
#include <format>
#include <fstream>
#include <iterator>

namespace krys::ui
{
  namespace
  {
    constexpr utf8_stringview ErrorDomain = u8"KrystalUI";
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

    const auto pathString = url.FileSystemPath();
    const std::filesystem::path path(pathString);
    std::ifstream file(path, std::ios::binary);
    if (!file)
    {
      completionHandler(
        Unexpected(boo::ResourceError(utf8_string(ErrorDomain), 0, url, u8"Couldn't open the file")));
      return;
    }
    List<uint8> bytes {std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>()};

    // The type is the extension's, without its dot.
    const auto extension = path.extension().u8string();
    const auto size = static_cast<int64>(bytes.size());
    boo::ResourceResponse response(url,
                                   boo::MIMETypeRegistry::MIMETypeForExtension(
                                     utf8_stringview(extension).substr(extension.empty() ? 0 : 1)),
                                   size);
    // The file's modification time is its document's lastModified, as an HTTP-date.
    std::error_code error;
    if (auto writeTime = std::filesystem::last_write_time(path, error); !error)
    {
      auto modified = std::chrono::floor<std::chrono::seconds>(
        std::chrono::clock_cast<std::chrono::system_clock>(writeTime));
      auto date = std::format("{:%a, %d %b %Y %H:%M:%S} GMT", modified);
      response.SetHTTPHeaderField(
        u8"Last-Modified", utf8_stringview(reinterpret_cast<const char8_t *>(date.data()), date.size()));
    }
    completionHandler(boo::LoadedResource {.Response = krys::move(response),
                                           .Data = boo::SharedBuffer::Create(krys::move(bytes))});
  }
}
