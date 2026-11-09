/*
 * Copyright (c) 2008-2009 Google Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following disclaimer
 * in the documentation and/or other materials provided with the
 * distribution.
 *     * Neither the name of Google Inc. nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "Krystal.Booey/Platform/ImageDecoders/BMPImageReader.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"

namespace krys::boo
{
  // This class decodes the BMP image format.
  class BMPImageDecoder final : public ScalableImageDecoder
  {
  private:
    // An index into |m_data| representing how much we've already decoded.
    // Note that this only tracks data _this_ class decodes; once the
    // BMPImageReader takes over this will not be updated further.
    size_t _decodedOffset {0};

    // The reader used to do most of the BMP decoding.
    UniquePtr<BMPImageReader> _reader;

  public:
    KRYS_NODISCARD static Ref<ScalableImageDecoder>
      Create(AlphaOption alphaOption, GammaAndColorProfileOption gammaAndColorProfileOption) noexcept
    {
      return AdoptRef(*new BMPImageDecoder(alphaOption, gammaAndColorProfileOption));
    }

    // ScalableImageDecoder
    KRYS_NODISCARD utf8_string FilenameExtension() const noexcept final
    {
      return u8"bmp";
    }
    void SetData(const SharedBuffer &data, bool allDataReceived) noexcept final;
    ScalableImageDecoderFrame *FrameBufferAtIndex(size_t index) noexcept final;
    // CAUTION: setFailed() deletes |m_reader|. Be careful to avoid
    // accessing deleted memory, especially when calling this from inside
    // BMPImageReader!
    bool SetFailed() noexcept final;

  private:
    BMPImageDecoder(AlphaOption, GammaAndColorProfileOption) noexcept;
    void TryDecodeSize(bool allDataReceived) noexcept final
    {
      Decode(true, allDataReceived);
    }

    KRYS_NODISCARD uint32 ReadUint32(size_t offset) const noexcept
    {
      return BMPImageReader::ReadUint32(*_data, _decodedOffset + offset);
    }

    // Decodes the image. If |onlySize| is true, stops decoding after
    // calculating the image size. If decoding fails but there is no more
    // data coming, sets the "decode failure" flag.
    void Decode(bool onlySize, bool allDataReceived) noexcept;

    // Decodes the image. If |onlySize| is true, stops decoding after
    // calculating the image size. Returns whether decoding succeeded.
    bool DecodeHelper(bool onlySize) noexcept;

    // Processes the file header at the beginning of the data. Sets
    // |*imgDataOffset| based on the header contents. Returns true if the
    // file header could be decoded.
    bool ProcessFileHeader(size_t *imgDataOffset) noexcept;
  };
}
