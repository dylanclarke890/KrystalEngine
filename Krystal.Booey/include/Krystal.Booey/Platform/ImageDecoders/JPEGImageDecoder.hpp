/*
 * Copyright (C) 2006 Apple Inc. All rights reserved.
 * Copyright (C) 2008-2009 Torch Mobile, Inc.
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
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "Krystal.Booey/Platform/ImageDecoders/ICCProfileTransform.hpp"
#include "Krystal.Booey/Platform/ImageDecoders/ScalableImageDecoder.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"

namespace krys::boo
{
  class JPEGImageReader;

  // This class decodes the JPEG image format.
  // @note libjpeg reports errors with longjmp, which on Windows unwinds as an exception would, so the
  // functions between the reader's setjmp and libjpeg's calls (writing scanlines) can't be noexcept.
  class JPEGImageDecoder final : public ScalableImageDecoder
  {
  private:
    UniquePtr<JPEGImageReader> _reader;
    // The conversion from the colour space of the image's ICC profile, and a CMYK row converted with it.
    ICCProfileTransform _iccTransform;
    List<uint8> _transformedRow;

  public:
    KRYS_NODISCARD static Ref<ScalableImageDecoder>
      Create(AlphaOption alphaOption, GammaAndColorProfileOption gammaAndColorProfileOption) noexcept
    {
      return AdoptRef(*new JPEGImageDecoder(alphaOption, gammaAndColorProfileOption));
    }

    ~JPEGImageDecoder() noexcept override;

    // ScalableImageDecoder
    KRYS_NODISCARD utf8_string FilenameExtension() const noexcept override
    {
      return u8"jpg";
    }
    ScalableImageDecoderFrame *FrameBufferAtIndex(size_t index) noexcept override;
    // CAUTION: setFailed() deletes |m_reader|.  Be careful to avoid
    // accessing deleted memory, especially when calling this from inside
    // JPEGImageReader!
    bool SetFailed() noexcept override;

    bool OutputScanlines();
    /// @brief Converts what's decoded from the colour space @p profile describes, laid out as @p layout
    /// (RGB rows are written as BGRA, and CMYK ones as inverted CMYK).
    void SetICCProfile(Span<const uint8> profile, ICCProfileTransform::PixelLayout layout) noexcept;
    void JpegComplete();

  private:
    JPEGImageDecoder(AlphaOption, GammaAndColorProfileOption) noexcept;
    void TryDecodeSize(bool allDataReceived) noexcept override
    {
      Decode(true, allDataReceived);
    }

    // Decodes the image.  If |onlySize| is true, stops decoding after
    // calculating the image size.  If decoding fails but there is no more
    // data coming, sets the "decode failure" flag.
    void Decode(bool onlySize, bool allDataReceived) noexcept;

    template <int TColorSpace>
    bool OutputScanlines(ScalableImageDecoderFrame &buffer);

    void Clear() noexcept;
  };
}
