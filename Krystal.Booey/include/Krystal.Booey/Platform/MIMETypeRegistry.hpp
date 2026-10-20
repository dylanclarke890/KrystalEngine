#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/String.hpp"

namespace krys::boo
{
  /// @brief Which MIME types Booey supports.
  // FIXME: The rest of the registry (extensions, non-image types), and SVG, AVIF and video types once they're
  // supported.
  struct MIMETypeRegistry
  {
    /// @brief Whether the image decoders support @p mimeType (PNG, JPEG, GIF, WebP, BMP and ICO), ignoring
    /// case.
    KRYS_NODISCARD static bool IsSupportedImageMIMEType(utf8_stringview mimeType) noexcept;

    /// @brief Whether an image, video or SVG of type @p mimeType can be shown.
    KRYS_NODISCARD static bool IsSupportedImageVideoOrSVGMIMEType(utf8_stringview mimeType) noexcept;
  };
}
