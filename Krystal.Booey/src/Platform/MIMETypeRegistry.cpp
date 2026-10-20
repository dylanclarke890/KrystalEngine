#include "Krystal.Booey/Platform/MIMETypeRegistry.hpp"
#include "Krystal.Core/Text/ASCII.hpp"
#include <algorithm>

namespace krys::boo
{
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
