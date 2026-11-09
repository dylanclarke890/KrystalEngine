/*
 * Copyright (C) 2006-2016 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/String.hpp"

namespace krys::boo
{
  /// @brief Which MIME types Booey supports.
  // FIXME: The rest of the registry (non-image types, extensions for types), and AVIF and video types once
  // they're supported.
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

    /// @brief Whether the image encoders support @p mimeType (PNG, JPEG and WebP), ignoring case.
    KRYS_NODISCARD static bool IsSupportedImageMIMETypeForEncoding(utf8_stringview mimeType) noexcept;

    /// @brief Whether @p mimeType is JPEG's, ignoring case.
    KRYS_NODISCARD static bool IsJPEGMIMEType(utf8_stringview mimeType) noexcept;

    /// @brief Whether an image, video or SVG of type @p mimeType can be shown.
    KRYS_NODISCARD static bool IsSupportedImageVideoOrSVGMIMEType(utf8_stringview mimeType) noexcept;

    /// @brief Whether @p mimeType is an XML type: `text/xml`, `application/xml` or a type ending in `+xml`,
    /// ignoring case.
    KRYS_NODISCARD static bool IsXMLMIMEType(utf8_stringview mimeType) noexcept;
  };
}
