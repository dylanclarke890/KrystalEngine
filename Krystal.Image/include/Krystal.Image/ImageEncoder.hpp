#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Geometry/Size.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/Span.hpp"

namespace krys::img
{
  /// @brief A format `EncodeImage` writes.
  enum class EncodedImageFormat : uint8
  {
    PNG,
    JPEG,
    /// @brief Lossy WebP.
    WebP,
  };

  /// @brief @p size's pixels, RGBA8 rows from @p rgba, encoded as @p format; empty if @p size is empty,
  /// @p rgba holds too few pixels, or encoding fails.
  /// @param quality From 0 to 1, for JPEG and WebP; the encoder's best if empty or out of range.
  /// @note A JPEG has no alpha: its pixels are the colours alone, so premultiplied pixels give what's drawn
  /// over opaque black.
  KRYS_NODISCARD List<uint8> EncodeImage(Span<const uint8> rgba, const SizeI &size, EncodedImageFormat format,
                                         Maybe<double> quality = {}) noexcept;
}
