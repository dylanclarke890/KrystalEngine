#include "Krystal.Booey/Platform/MIMETypeRegistry.hpp"
#include "Krystal.Core/Text/ASCII.hpp"
#include <algorithm>

namespace krys::boo
{
  // FIXME: The system's types for other extensions (the registry's content types on Windows), through the
  // host.
  utf8_string MIMETypeRegistry::MIMETypeForExtension(utf8_stringview extension) noexcept
  {
    struct Mapping
    {
      utf8_stringview Extension;
      utf8_stringview Type;
    };
    constexpr Mapping mimetypeMap[] = {
      {u8"txt", u8"text/plain"},
      {u8"pdf", u8"application/pdf"},
      {u8"ps", u8"application/postscript"},
      {u8"css", u8"text/css"},
      {u8"html", u8"text/html"},
      {u8"htm", u8"text/html"},
      {u8"xml", u8"text/xml"},
      {u8"xsl", u8"text/xsl"},
      {u8"js", u8"application/x-javascript"},
      {u8"xht", u8"application/xhtml+xml"},
      {u8"xhtml", u8"application/xhtml+xml"},
      {u8"rss", u8"application/rss+xml"},
      {u8"webarchive", u8"application/x-webarchive"},
      {u8"svg", u8"image/svg+xml"},
      {u8"svgz", u8"image/svg+xml"},
      {u8"jpg", u8"image/jpeg"},
      {u8"jpeg", u8"image/jpeg"},
      {u8"png", u8"image/png"},
      {u8"tif", u8"image/tiff"},
      {u8"tiff", u8"image/tiff"},
      {u8"ico", u8"image/ico"},
      {u8"cur", u8"image/ico"},
      {u8"bmp", u8"image/bmp"},
      {u8"wml", u8"text/vnd.wap.wml"},
      {u8"wmlc", u8"application/vnd.wap.wmlc"},
      {u8"m4a", u8"audio/x-m4a"},
    };
    if (extension.empty())
    {
      return {};
    }
    for (auto &mapping : mimetypeMap)
    {
      if (text::IsASCIICaselessEqual(extension, mapping.Extension))
      {
        return utf8_string(mapping.Type);
      }
    }
    return {};
  }

  bool MIMETypeRegistry::IsSupportedImageMIMEType(utf8_stringview mimeType) noexcept
  {
    constexpr utf8_stringview supportedTypes[] = {
      u8"image/png",
      u8"image/apng",
      u8"image/jpeg",
      u8"image/jpg",
      u8"image/pjpeg",
      u8"image/gif",
      u8"image/bmp",
      u8"image/x-bmp",
      u8"image/webp",
      u8"image/x-icon",
      u8"image/vnd.microsoft.icon",
    };
    return std::ranges::any_of(supportedTypes,
                               [&](auto type) { return text::IsASCIICaselessEqual(mimeType, type); });
  }

  bool MIMETypeRegistry::IsSupportedImageVideoOrSVGMIMEType(utf8_stringview mimeType) noexcept
  {
    return IsSupportedImageMIMEType(mimeType);
  }
}
