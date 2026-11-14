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
#include "Krystal.Core/Colour/ColourConversion.hpp"
#include "Krystal.Core/Colour/ColourTypes.hpp"
#include "Krystal.Core/Types/Array.hpp"
#include <algorithm>
#include <cmath>
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

    /// @brief The components of @p colour, converted to @p destination.
    template <typename TColour>
    KRYS_NODISCARD Array<float, 3> ConvertFrom(const TColour &colour,
                                               DestinationColorSpace::Space destination) noexcept
    {
      auto components = [](const auto &converted)
      {
        auto [red, green, blue, alpha] = converted.Resolved();
        (void)alpha;
        return Array<float, 3> {red, green, blue};
      };
      switch (destination)
      {
        case DestinationColorSpace::Space::SRGB: return components(ConvertColour<SRGBA<float>>(colour));
        case DestinationColorSpace::Space::LinearSRGB:
          return components(ConvertColour<LinearSRGBA<float>>(colour));
        case DestinationColorSpace::Space::DisplayP3:
          return components(ConvertColour<DisplayP3<float>>(colour));
      }
      return components(ConvertColour<SRGBA<float>>(colour));
    }

    /// @brief @p rgb, a straight colour in @p source, in @p destination.
    KRYS_NODISCARD Array<float, 3> ConvertColorSpace(Array<float, 3> rgb, DestinationColorSpace::Space source,
                                                     DestinationColorSpace::Space destination) noexcept
    {
      switch (source)
      {
        case DestinationColorSpace::Space::SRGB:
          return ConvertFrom(SRGBA<float> {rgb[0], rgb[1], rgb[2], 1.f}, destination);
        case DestinationColorSpace::Space::LinearSRGB:
          return ConvertFrom(LinearSRGBA<float> {rgb[0], rgb[1], rgb[2], 1.f}, destination);
        case DestinationColorSpace::Space::DisplayP3:
          return ConvertFrom(DisplayP3<float> {rgb[0], rgb[1], rgb[2], 1.f}, destination);
      }
      return rgb;
    }

    /// @brief Converts pixels between colour spaces: each is unpremultiplied, converted with the
    /// 'relative-colorimetric' intent (colours outside the destination's gamut are clipped to it), and
    /// premultiplied again if the destination is.
    void ConvertImagePixelsBetweenColorSpaces(const ConstPixelBufferConversionView &source,
                                              const PixelBufferConversionView &destination,
                                              const IntSize &destinationSize) noexcept
    {
      const bool sourceIsBGR = source.Format.Format != PixelFormat::RGBA8;
      const bool destinationIsBGR = destination.Format.Format != PixelFormat::RGBA8;
      const bool sourceIsOpaque = source.Format.Format == PixelFormat::BGRX8;
      const bool destinationIsOpaque = destination.Format.Format == PixelFormat::BGRX8;
      const bool sourceIsPremultiplied = source.Format.AlphaFormat == AlphaPremultiplication::Premultiplied;
      const bool destinationIsPremultiplied =
        destination.Format.AlphaFormat == AlphaPremultiplication::Premultiplied;
      const auto sourceSpace = source.Format.ColorSpace.GetSpace();
      const auto destinationSpace = destination.Format.ColorSpace.GetSpace();

      for (int32 y = 0; y < destinationSize.Height(); ++y)
      {
        auto sourceRow = source.Rows.subspan(static_cast<size_t>(source.BytesPerRow) * y);
        auto destinationRow = destination.Rows.subspan(static_cast<size_t>(destination.BytesPerRow) * y);
        for (int32 x = 0; x < destinationSize.Width(); ++x)
        {
          const uint8 *in = sourceRow.data() + static_cast<size_t>(x) * 4u;
          uint8 *out = destinationRow.data() + static_cast<size_t>(x) * 4u;
          const uint8 alpha = sourceIsOpaque ? 255 : in[3];
          if (!alpha)
          {
            std::memset(out, 0, 4);
            if (destinationIsOpaque)
            {
              out[3] = 255;
            }
            continue;
          }
          const float scale = sourceIsPremultiplied ? 1.f / alpha : 1.f / 255.f;
          Array<float, 3> rgb {in[sourceIsBGR ? 2 : 0] * scale, in[1] * scale,
                               in[sourceIsBGR ? 0 : 2] * scale};
          rgb = ConvertColorSpace(rgb, sourceSpace, destinationSpace);

          const uint8 outAlpha = destinationIsOpaque ? 255 : alpha;
          const float outScale = destinationIsPremultiplied ? static_cast<float>(outAlpha) : 255.f;
          auto encode = [&](float component)
          {
            return static_cast<uint8>(std::lround(std::clamp(component, 0.f, 1.f) * outScale));
          };
          out[destinationIsBGR ? 2 : 0] = encode(rgb[0]);
          out[1] = encode(rgb[1]);
          out[destinationIsBGR ? 0 : 2] = encode(rgb[2]);
          out[3] = outAlpha;
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

    if (source.Format.ColorSpace != destination.Format.ColorSpace)
    {
      ConvertImagePixelsBetweenColorSpaces(source, destination, destinationSize);
      return;
    }

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
