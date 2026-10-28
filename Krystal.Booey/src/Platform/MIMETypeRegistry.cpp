#include "Krystal.Booey/Platform/MIMETypeRegistry.hpp"
#include "Krystal.Core/Text/ASCII.hpp"
#include <algorithm>

namespace krys::boo
{
  namespace
  {
    // https://www.rfc-editor.org/rfc/rfc7230#section-3.2.6
    KRYS_NODISCARD bool IsValidHTTPToken(utf8_stringview value) noexcept
    {
      if (value.empty())
      {
        return false;
      }
      return std::ranges::all_of(value,
                                 [](char8 c)
                                 {
                                   return text::IsASCIIAlphanumeric(c)
                                          || utf8_stringview(u8"!#$%&'*+-.^_`|~").find(c)
                                               != utf8_stringview::npos;
                                 });
    }

    KRYS_NODISCARD bool IsValidMIMETypeWithSuffix(utf8_stringview mimeType, utf8_stringview suffix) noexcept
    {
      if (mimeType.size() < suffix.size()
          || !text::IsASCIICaselessEqual(mimeType.substr(mimeType.size() - suffix.size()), suffix))
      {
        return false;
      }

      if (mimeType.size() < suffix.size() + 2)
      {
        return false;
      }

      auto slashPosition = mimeType.find(u8'/');
      if (slashPosition == utf8_stringview::npos || slashPosition <= 0)
      {
        return false;
      }

      auto type = mimeType.substr(0, slashPosition);
      if (!IsValidHTTPToken(type))
      {
        return false;
      }

      // Assume the suffix is already a valid HTTP token and avoid checking it.
      auto subtypeExcludingSuffix =
        mimeType.substr(slashPosition + 1, mimeType.size() - slashPosition - 1 - suffix.size());
      return IsValidHTTPToken(subtypeExcludingSuffix);
    }
  }

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

  bool MIMETypeRegistry::IsSupportedImageMIMETypeForEncoding(utf8_stringview mimeType) noexcept
  {
    constexpr utf8_stringview supportedTypes[] = {
      u8"image/png",
      u8"image/jpeg",
      u8"image/webp",
    };
    return std::ranges::any_of(supportedTypes,
                               [&](auto type) { return text::IsASCIICaselessEqual(mimeType, type); });
  }

  bool MIMETypeRegistry::IsJPEGMIMEType(utf8_stringview mimeType) noexcept
  {
    return text::IsASCIICaselessEqual(mimeType, utf8_stringview(u8"image/jpeg"))
           || text::IsASCIICaselessEqual(mimeType, utf8_stringview(u8"image/jpg"));
  }

  bool MIMETypeRegistry::IsSupportedImageVideoOrSVGMIMEType(utf8_stringview mimeType) noexcept
  {
    return IsSupportedImageMIMEType(mimeType)
           || text::IsASCIICaselessEqual(mimeType, utf8_stringview(u8"image/svg+xml"));
  }

  bool MIMETypeRegistry::IsXMLMIMEType(utf8_stringview mimeType) noexcept
  {
    if (text::IsASCIICaselessEqual(mimeType, utf8_stringview(u8"text/xml"))
        || text::IsASCIICaselessEqual(mimeType, utf8_stringview(u8"application/xml")))
    {
      return true;
    }

    return IsValidMIMETypeWithSuffix(mimeType, u8"+xml");
  }
}
