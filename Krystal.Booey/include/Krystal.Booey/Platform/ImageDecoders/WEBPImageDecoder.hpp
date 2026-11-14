/*
 * Copyright (C) 2010 Google Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1.  Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 * 2.  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 * 3.  Neither the name of Apple Inc. ("Apple") nor the names of
 *     its contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE AND ITS CONTRIBUTORS "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL APPLE OR ITS CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "Krystal.Booey/Platform/ImageDecoders/ICCProfileTransform.hpp"
#include "Krystal.Booey/Platform/ImageDecoders/ScalableImageDecoder.hpp"
#include "Krystal.Core/Types/List.hpp"

struct WebPDemuxer;
struct WebPIterator;
struct WebPIDecoder;
struct WebPDecBuffer;

namespace krys::boo
{
  /// @brief Decodes WebP images, still and animated, with libwebp.
  class WEBPImageDecoder final : public ScalableImageDecoder
  {
  private:
    int32 _repetitionCount {0};
    size_t _frameCount {0};
    int32 _formatFlags {0};
    bool _headerParsed {false};
    // The conversion from the colour space of the image's ICCP profile (read with the first frame it
    // decodes, since the profile comes before them), and a row converted with it.
    bool _iccProfileRead {false};
    ICCProfileTransform _iccTransform;
    List<uint8> _transformedRow;

  public:
    KRYS_NODISCARD static Ref<ScalableImageDecoder>
      Create(AlphaOption alphaOption, GammaAndColorProfileOption gammaAndColorProfileOption) noexcept
    {
      return AdoptRef(*new WEBPImageDecoder(alphaOption, gammaAndColorProfileOption));
    }

    ~WEBPImageDecoder() noexcept override;

    KRYS_NODISCARD utf8_string FilenameExtension() const noexcept override
    {
      return u8"webp";
    }
    void SetData(const SharedBuffer &data, bool allDataReceived) noexcept final;
    ScalableImageDecoderFrame *FrameBufferAtIndex(size_t index) noexcept override;
    KRYS_NODISCARD RepetitionCount GetRepetitionCount() const noexcept override;
    KRYS_NODISCARD size_t FrameCount() const noexcept override
    {
      return _frameCount;
    }
    void ClearFrameBufferCache(size_t clearBeforeFrame) noexcept override;

  private:
    WEBPImageDecoder(AlphaOption, GammaAndColorProfileOption) noexcept;
    void TryDecodeSize(bool) noexcept override
    {
      ParseHeader();
    }
    void Decode(size_t frameIndex, bool allDataReceived) noexcept;
    void DecodeFrame(size_t frameIndex, WebPDemuxer *demuxer) noexcept;
    void ParseHeader() noexcept;
    void ReadICCProfile(WebPDemuxer *demuxer) noexcept;
    bool InitFrameBuffer(size_t frameIndex, const WebPIterator *webpFrame) noexcept;
    void ApplyPostProcessing(size_t frameIndex, WebPIDecoder *decoder, WebPDecBuffer &decoderBuffer,
                             bool blend) noexcept;
    KRYS_NODISCARD size_t FindFirstRequiredFrameToDecode(size_t frameIndex, WebPDemuxer *demuxer) noexcept;
  };
}
