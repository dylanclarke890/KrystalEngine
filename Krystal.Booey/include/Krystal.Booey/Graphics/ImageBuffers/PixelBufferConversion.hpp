/*
 * Copyright (C) 2021 Apple Inc. All rights reserved.
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
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "Krystal.Booey/Graphics/ImageBuffers/PixelBufferFormat.hpp"
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
  /// unpremultiplying their alpha, swapping red and blue, and converting between colour spaces (clipping to
  /// the destination's gamut) as needed.
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
