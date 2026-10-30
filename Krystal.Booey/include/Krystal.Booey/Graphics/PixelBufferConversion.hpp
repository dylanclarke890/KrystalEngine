#pragma once

#include "Krystal.Booey/Graphics/PixelBufferFormat.hpp"
#include "Krystal.Core/Assert.hpp"
#include "Krystal.Core/Types/Span.hpp"

namespace krys::boo
{
  class IntSize;

  /// @brief Rows of pixels to convert into.
  struct PixelBufferConversionView
  {
    PixelBufferFormat Format;
    uint32 BytesPerRow;
    Span<uint8> Rows;
  };

  /// @brief Rows of pixels to convert from.
  struct ConstPixelBufferConversionView
  {
    PixelBufferFormat Format;
    uint32 BytesPerRow;
    Span<const uint8> Rows;
  };

  /// @brief Converts @p destinationSize pixels from @p source's format to @p destination's: premultiplying or
  /// unpremultiplying their alpha and swapping red and blue as needed.
  /// FIXME: Converting between colour spaces; both views must be in the same one.
  void ConvertImagePixels(const ConstPixelBufferConversionView &source,
                          const PixelBufferConversionView &destination,
                          const IntSize &destinationSize) noexcept;

  void CopyRowsInternal(uint32 sourceBytesPerRow, Span<const uint8> source, uint32 destinationBytesPerRow,
                        Span<uint8> destination, uint32 rows, uint32 copyBytesPerRow) noexcept;

  inline void CopyRows(uint32 sourceBytesPerRow, Span<const uint8> source, uint32 destinationBytesPerRow,
                       Span<uint8> destination, uint32 rows, uint32 copyBytesPerRow) noexcept
  {
    if (!rows || !copyBytesPerRow)
    {
      return;
    }
    uint32 requiredSourceBytes = sourceBytesPerRow * (rows - 1) + copyBytesPerRow;
    if (source.size() < requiredSourceBytes)
    {
      krys_debug_assert(false);
      return;
    }
    uint32 requiredDestinationBytes = destinationBytesPerRow * (rows - 1) + copyBytesPerRow;
    if (destination.size() < requiredDestinationBytes)
    {
      krys_debug_assert(false);
      return;
    }
    if (rows > 1 && (sourceBytesPerRow < copyBytesPerRow || destinationBytesPerRow < copyBytesPerRow))
    {
      krys_debug_assert(false);
      return;
    }
    CopyRowsInternal(sourceBytesPerRow, source, destinationBytesPerRow, destination, rows, copyBytesPerRow);
  }
}
