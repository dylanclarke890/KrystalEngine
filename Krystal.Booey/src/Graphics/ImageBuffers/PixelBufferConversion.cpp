/*
 * Copyright (C) 2021-2023 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/Graphics/ImageBuffers/PixelBufferConversion.hpp"
#include "Krystal.Booey/Graphics/Geometry/IntSize.hpp"
#include <cstring>

namespace krys::boo
{
  namespace
  {
    enum class PixelFormatConversion : uint8
    {
      None,
      Permute
    };

    using SourcePixel = Span<const uint8, 4>;
    using DestinationPixel = Span<uint8, 4>;

    template <PixelFormatConversion TConversion>
    void ConvertSinglePixelPremultipliedToPremultiplied(SourcePixel sourcePixel,
                                                        DestinationPixel destinationPixel) noexcept
    {
      uint8 alpha = sourcePixel[3];
      if (!alpha)
      {
        std::memset(destinationPixel.data(), 0, 4);
        return;
      }

      if constexpr (TConversion == PixelFormatConversion::None)
      {
        std::memcpy(destinationPixel.data(), sourcePixel.data(), 4);
      }
      else
      {
        // Swap pixel channels BGRA <-> RGBA.
        destinationPixel[0] = sourcePixel[2];
        destinationPixel[1] = sourcePixel[1];
        destinationPixel[2] = sourcePixel[0];
        destinationPixel[3] = sourcePixel[3];
      }
    }

    template <PixelFormatConversion TConversion>
    void ConvertSinglePixelPremultipliedToUnpremultiplied(SourcePixel sourcePixel,
                                                          DestinationPixel destinationPixel) noexcept
    {
      uint8 alpha = sourcePixel[3];
      if (!alpha || alpha == 255)
      {
        ConvertSinglePixelPremultipliedToPremultiplied<TConversion>(sourcePixel, destinationPixel);
        return;
      }

      if constexpr (TConversion == PixelFormatConversion::None)
      {
        destinationPixel[0] = static_cast<uint8>((sourcePixel[0] * 255) / alpha);
        destinationPixel[1] = static_cast<uint8>((sourcePixel[1] * 255) / alpha);
        destinationPixel[2] = static_cast<uint8>((sourcePixel[2] * 255) / alpha);
        destinationPixel[3] = alpha;
      }
      else
      {
        // Swap pixel channels BGRA <-> RGBA.
        destinationPixel[0] = static_cast<uint8>((sourcePixel[2] * 255) / alpha);
        destinationPixel[1] = static_cast<uint8>((sourcePixel[1] * 255) / alpha);
        destinationPixel[2] = static_cast<uint8>((sourcePixel[0] * 255) / alpha);
        destinationPixel[3] = alpha;
      }
    }

    template <PixelFormatConversion TConversion>
    void ConvertSinglePixelUnpremultipliedToPremultiplied(SourcePixel sourcePixel,
                                                          DestinationPixel destinationPixel) noexcept
    {
      uint8 alpha = sourcePixel[3];
      if (!alpha || alpha == 255)
      {
        ConvertSinglePixelPremultipliedToPremultiplied<TConversion>(sourcePixel, destinationPixel);
        return;
      }

      if constexpr (TConversion == PixelFormatConversion::None)
      {
        destinationPixel[0] = static_cast<uint8>((sourcePixel[0] * alpha + 254) / 255);
        destinationPixel[1] = static_cast<uint8>((sourcePixel[1] * alpha + 254) / 255);
        destinationPixel[2] = static_cast<uint8>((sourcePixel[2] * alpha + 254) / 255);
        destinationPixel[3] = alpha;
      }
      else
      {
        // Swap pixel channels BGRA <-> RGBA.
        destinationPixel[0] = static_cast<uint8>((sourcePixel[2] * alpha + 254) / 255);
        destinationPixel[1] = static_cast<uint8>((sourcePixel[1] * alpha + 254) / 255);
        destinationPixel[2] = static_cast<uint8>((sourcePixel[0] * alpha + 254) / 255);
        destinationPixel[3] = alpha;
      }
    }

    template <PixelFormatConversion TConversion>
    void ConvertSinglePixelUnpremultipliedToUnpremultiplied(SourcePixel sourcePixel,
                                                            DestinationPixel destinationPixel) noexcept
    {
      if constexpr (TConversion == PixelFormatConversion::None)
      {
        std::memcpy(destinationPixel.data(), sourcePixel.data(), 4);
      }
      else
      {
        // Swap pixel channels BGRA <-> RGBA.
        destinationPixel[0] = sourcePixel[2];
        destinationPixel[1] = sourcePixel[1];
        destinationPixel[2] = sourcePixel[0];
        destinationPixel[3] = sourcePixel[3];
      }
    }

    template <void (*TConvert)(SourcePixel, DestinationPixel) noexcept>
    void ConvertImagePixelsUnaccelerated(const ConstPixelBufferConversionView &source,
                                         const PixelBufferConversionView &destination,
                                         const IntSize &destinationSize) noexcept
    {
      size_t bytesPerRow = static_cast<size_t>(destinationSize.Width()) * 4;
      for (int32 y = 0; y < destinationSize.Height(); ++y)
      {
        auto sourceRow = source.Rows.subspan(static_cast<size_t>(source.BytesPerRow) * y);
        auto destinationRow = destination.Rows.subspan(static_cast<size_t>(destination.BytesPerRow) * y);
        for (size_t x = 0; x < bytesPerRow; x += 4)
        {
          TConvert(sourceRow.subspan(x).first<4>(), destinationRow.subspan(x).first<4>());
        }
      }
    }

    void CopyImagePixels(const ConstPixelBufferConversionView &source,
                         const PixelBufferConversionView &destination,
                         const IntSize &destinationSize) noexcept
    {
      size_t bytesPerRow = static_cast<size_t>(destinationSize.Width()) * 4;
      if (!bytesPerRow || destinationSize.Height() <= 0)
      {
        return;
      }

      if (bytesPerRow == source.BytesPerRow && bytesPerRow == destination.BytesPerRow)
      {
        std::memcpy(destination.Rows.data(), source.Rows.data(), bytesPerRow * destinationSize.Height());
        return;
      }

      for (int32 y = 0; y < destinationSize.Height(); ++y)
      {
        auto sourceRow = source.Rows.subspan(static_cast<size_t>(source.BytesPerRow) * y);
        auto destinationRow = destination.Rows.subspan(static_cast<size_t>(destination.BytesPerRow) * y);
        std::memcpy(destinationRow.data(), sourceRow.data(), bytesPerRow);
      }
    }
  }

  void ConvertImagePixels(const ConstPixelBufferConversionView &source,
                          const PixelBufferConversionView &destination,
                          const IntSize &destinationSize) noexcept
  {
    // We currently only support converting between RGBA8, BGRA8, and BGRX8.
    krys_debug_assert(source.Format.Format == PixelFormat::RGBA8 || source.Format.Format == PixelFormat::BGRA8
                      || source.Format.Format == PixelFormat::BGRX8);
    krys_debug_assert(destination.Format.Format == PixelFormat::RGBA8
                      || destination.Format.Format == PixelFormat::BGRA8
                      || destination.Format.Format == PixelFormat::BGRX8);

    if (source.Format.AlphaFormat == destination.Format.AlphaFormat
        && source.Format.Format == destination.Format.Format
        && source.Format.ColorSpace == destination.Format.ColorSpace)
    {
      CopyImagePixels(source, destination, destinationSize);
      return;
    }

    // FIXME: We don't currently support converting pixel data between different color spaces in the
    // non-accelerated path. This could be added using conversion functions from ColorConversion.hpp.
    krys_debug_assert(source.Format.ColorSpace == destination.Format.ColorSpace);

    using enum PixelFormatConversion;
    if (source.Format.AlphaFormat == destination.Format.AlphaFormat)
    {
      if (source.Format.Format == destination.Format.Format)
      {
        if (source.Format.AlphaFormat == AlphaPremultiplication::Premultiplied)
        {
          ConvertImagePixelsUnaccelerated<ConvertSinglePixelPremultipliedToPremultiplied<None>>(
            source, destination, destinationSize);
        }
        else
        {
          ConvertImagePixelsUnaccelerated<ConvertSinglePixelUnpremultipliedToUnpremultiplied<None>>(
            source, destination, destinationSize);
        }
      }
      else
      {
        if (destination.Format.AlphaFormat == AlphaPremultiplication::Premultiplied)
        {
          ConvertImagePixelsUnaccelerated<ConvertSinglePixelPremultipliedToPremultiplied<Permute>>(
            source, destination, destinationSize);
        }
        else
        {
          ConvertImagePixelsUnaccelerated<ConvertSinglePixelUnpremultipliedToUnpremultiplied<Permute>>(
            source, destination, destinationSize);
        }
      }
    }
    else
    {
      if (source.Format.Format == destination.Format.Format)
      {
        if (source.Format.AlphaFormat == AlphaPremultiplication::Premultiplied)
        {
          ConvertImagePixelsUnaccelerated<ConvertSinglePixelPremultipliedToUnpremultiplied<None>>(
            source, destination, destinationSize);
        }
        else
        {
          ConvertImagePixelsUnaccelerated<ConvertSinglePixelUnpremultipliedToPremultiplied<None>>(
            source, destination, destinationSize);
        }
      }
      else
      {
        if (destination.Format.AlphaFormat == AlphaPremultiplication::Premultiplied)
        {
          ConvertImagePixelsUnaccelerated<ConvertSinglePixelUnpremultipliedToPremultiplied<Permute>>(
            source, destination, destinationSize);
        }
        else
        {
          ConvertImagePixelsUnaccelerated<ConvertSinglePixelPremultipliedToUnpremultiplied<Permute>>(
            source, destination, destinationSize);
        }
      }
    }
  }

  void CopyRowsInternal(uint32 sourceBytesPerRow, Span<const uint8> source, uint32 destinationBytesPerRow,
                        Span<uint8> destination, uint32 rows, uint32 copyBytesPerRow) noexcept
  {
    if (sourceBytesPerRow == destinationBytesPerRow && copyBytesPerRow == sourceBytesPerRow)
    {
      std::memcpy(destination.data(), source.data(), static_cast<size_t>(copyBytesPerRow) * rows);
      return;
    }
    for (uint32 row = 0; row < rows; ++row)
    {
      std::memcpy(destination.data(), source.data(), copyBytesPerRow);
      if (sourceBytesPerRow > source.size() || destinationBytesPerRow > destination.size())
      {
        break;
      }
      source = source.subspan(sourceBytesPerRow);
      destination = destination.subspan(destinationBytesPerRow);
    }
  }
}
