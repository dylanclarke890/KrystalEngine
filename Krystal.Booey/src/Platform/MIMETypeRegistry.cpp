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

  utf8_string MIMETypeRegistry::MIMETypeForExtension(utf8_stringview extension) noexcept
  {
    // The types of common extensions, after Chrome's built-in mappings (net::GetMimeTypeFromExtension),
    // without the system's types for other extensions (see DIFFERENCES.md).
    struct Mapping
    {
      utf8_stringview Extension;
      utf8_stringview Type;
    };
    static constexpr Mapping mappings[] = {
      {u8"html", u8"text/html"},
      {u8"htm", u8"text/html"},
      {u8"shtml", u8"text/html"},
      {u8"shtm", u8"text/html"},
      {u8"xhtml", u8"application/xhtml+xml"},
      {u8"xht", u8"application/xhtml+xml"},
      {u8"xhtm", u8"application/xhtml+xml"},
      {u8"css", u8"text/css"},
      {u8"js", u8"text/javascript"},
      {u8"mjs", u8"text/javascript"},
      {u8"json", u8"application/json"},
      {u8"wasm", u8"application/wasm"},
      {u8"xml", u8"text/xml"},
      {u8"xsl", u8"text/xml"},
      {u8"xslt", u8"text/xml"},
      {u8"txt", u8"text/plain"},
      {u8"text", u8"text/plain"},
      {u8"csv", u8"text/csv"},
      {u8"md", u8"text/markdown"},
      {u8"ics", u8"text/calendar"},
      {u8"sh", u8"text/x-sh"},
      {u8"rss", u8"application/rss+xml"},
      {u8"atom", u8"application/atom+xml"},
      {u8"rdf", u8"application/rdf+xml"},
      {u8"eml", u8"message/rfc822"},
      {u8"mht", u8"multipart/related"},
      {u8"mhtml", u8"multipart/related"},
      {u8"png", u8"image/png"},
      {u8"apng", u8"image/apng"},
      {u8"jpg", u8"image/jpeg"},
      {u8"jpeg", u8"image/jpeg"},
      {u8"jfif", u8"image/jpeg"},
      {u8"pjpeg", u8"image/jpeg"},
      {u8"pjp", u8"image/jpeg"},
      {u8"gif", u8"image/gif"},
      {u8"webp", u8"image/webp"},
      {u8"avif", u8"image/avif"},
      {u8"svg", u8"image/svg+xml"},
      {u8"svgz", u8"image/svg+xml"},
      {u8"bmp", u8"image/bmp"},
      {u8"ico", u8"image/x-icon"},
      {u8"cur", u8"image/x-icon"},
      {u8"tif", u8"image/tiff"},
      {u8"tiff", u8"image/tiff"},
      {u8"xbm", u8"image/x-xbitmap"},
      {u8"ttf", u8"font/ttf"},
      {u8"otf", u8"font/otf"},
      {u8"ttc", u8"font/collection"},
      {u8"woff", u8"font/woff"},
      {u8"woff2", u8"font/woff2"},
      {u8"mp3", u8"audio/mpeg"},
      {u8"wav", u8"audio/wav"},
      {u8"flac", u8"audio/flac"},
      {u8"ogg", u8"audio/ogg"},
      {u8"oga", u8"audio/ogg"},
      {u8"opus", u8"audio/ogg"},
      {u8"m4a", u8"audio/x-m4a"},
      {u8"weba", u8"audio/webm"},
      {u8"mp4", u8"video/mp4"},
      {u8"m4v", u8"video/mp4"},
      {u8"webm", u8"video/webm"},
      {u8"ogv", u8"video/ogg"},
      {u8"ogm", u8"video/ogg"},
      {u8"mpeg", u8"video/mpeg"},
      {u8"mpg", u8"video/mpeg"},
      {u8"pdf", u8"application/pdf"},
      {u8"ps", u8"application/postscript"},
      {u8"eps", u8"application/postscript"},
      {u8"ai", u8"application/postscript"},
      {u8"rtf", u8"application/rtf"},
      {u8"epub", u8"application/epub+zip"},
      {u8"zip", u8"application/zip"},
      {u8"gz", u8"application/gzip"},
      {u8"tgz", u8"application/gzip"},
      {u8"tar", u8"application/x-tar"},
      {u8"bin", u8"application/octet-stream"},
      {u8"exe", u8"application/octet-stream"},
      {u8"doc", u8"application/msword"},
      {u8"dot", u8"application/msword"},
      {u8"docx", u8"application/vnd.openxmlformats-officedocument.wordprocessingml.document"},
      {u8"xls", u8"application/vnd.ms-excel"},
      {u8"xlsx", u8"application/vnd.openxmlformats-officedocument.spreadsheetml.sheet"},
      {u8"ppt", u8"application/vnd.ms-powerpoint"},
      {u8"pptx", u8"application/vnd.openxmlformats-officedocument.presentationml.presentation"},
      {u8"cer", u8"application/x-x509-ca-cert"},
      {u8"crt", u8"application/x-x509-ca-cert"},
      {u8"p7m", u8"application/pkcs7-mime"},
      {u8"p7c", u8"application/pkcs7-mime"},
      {u8"p7s", u8"application/pkcs7-signature"},
    };
    if (extension.empty())
    {
      return {};
    }
    for (auto &mapping : mappings)
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
