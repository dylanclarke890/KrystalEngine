#include "Krystal.UI/FileLoaderStrategy.hpp"
#include "Krystal.Core/Text/ASCII.hpp"
#include <algorithm>
#include <filesystem>
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

    const auto extension = path.extension().u8string();
    const auto size = static_cast<int64>(bytes.size());
    completionHandler(boo::LoadedResource {
      .Response = boo::ResourceResponse(url, MimeTypeForExtension(utf8_stringview(extension)), size),
      .Data = boo::SharedBuffer::Create(krys::move(bytes))});
  }

  utf8_string FileLoaderStrategy::MimeTypeForExtension(utf8_stringview extension) noexcept
  {
    utf8_string lowered(extension);
    std::ranges::transform(lowered, lowered.begin(), [](char8_t c) { return text::ToASCIILower(c); });

    constexpr std::pair<utf8_stringview, utf8_stringview> types[] = {
      {u8".png", u8"image/png"},     {u8".apng", u8"image/apng"},     {u8".jpg", u8"image/jpeg"},
      {u8".jpeg", u8"image/jpeg"},   {u8".gif", u8"image/gif"},       {u8".webp", u8"image/webp"},
      {u8".bmp", u8"image/bmp"},     {u8".ico", u8"image/x-icon"},    {u8".cur", u8"image/x-icon"},
      {u8".svg", u8"image/svg+xml"}, {u8".css", u8"text/css"},        {u8".html", u8"text/html"},
      {u8".htm", u8"text/html"},     {u8".txt", u8"text/plain"},      {u8".ttf", u8"font/ttf"},
      {u8".otf", u8"font/otf"},      {u8".ttc", u8"font/collection"}, {u8".woff", u8"font/woff"},
      {u8".woff2", u8"font/woff2"},
    };
    for (const auto &[candidate, type] : types)
    {
      if (lowered == candidate)
      {
        return utf8_string(type);
      }
    }
    return {};
  }
}
