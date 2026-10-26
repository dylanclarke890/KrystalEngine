#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/String.hpp"

namespace krys::boo
{
  /// @brief Which MIME types Booey supports.
  // FIXME: The rest of the registry (non-image types, extensions for types), and SVG, AVIF and video types
  // once they're supported.
  struct MIMETypeRegistry
  {
    /// @brief The MIME type of files with @p extension (without its dot, ignoring case), or empty if it isn't
    /// known.
    KRYS_NODISCARD static utf8_string MIMETypeForExtension(utf8_stringview extension) noexcept;

    /// @brief The type of data whose type isn't known.
    KRYS_NODISCARD static utf8_stringview DefaultMIMEType() noexcept
    {
      return u8"application/octet-stream";
    }

    /// @brief Whether the image decoders support @p mimeType (PNG, JPEG, GIF, WebP, BMP and ICO), ignoring
    /// case.
    KRYS_NODISCARD static bool IsSupportedImageMIMEType(utf8_stringview mimeType) noexcept;

    /// @brief Whether an image, video or SVG of type @p mimeType can be shown.
    KRYS_NODISCARD static bool IsSupportedImageVideoOrSVGMIMEType(utf8_stringview mimeType) noexcept;
  };
}
