#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/Span.hpp"
#include "Krystal.Core/Types/String.hpp"

namespace krys::boo
{
  class PixelBuffer;

  /// @brief @p pixelBuffer, whose pixels are RGBA8, encoded as @p mimeType (PNG, JPEG or WebP); empty if
  /// the type isn't one of those or encoding fails.
  /// @param quality From 0 to 1, for JPEG and WebP; the encoder's best if empty or out of range.
  /// @note A JPEG has no alpha: its pixels are the colours alone, so a premultiplied buffer gives what's
  /// drawn over opaque black.
  KRYS_NODISCARD List<uint8> EncodeData(const PixelBuffer &pixelBuffer, utf8_stringview mimeType,
                                        Maybe<double> quality = {}) noexcept;

  /// @brief @p data as a `data:` URL of type @p mimeType, in base 64; `data:,` if it's empty.
  KRYS_NODISCARD utf8_string DataURL(Span<const uint8> data, utf8_stringview mimeType) noexcept;
}
