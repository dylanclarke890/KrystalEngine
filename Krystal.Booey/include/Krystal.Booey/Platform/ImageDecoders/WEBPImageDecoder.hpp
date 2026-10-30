#pragma once

#include "Krystal.Booey/Platform/ImageDecoders/ScalableImageDecoder.hpp"

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
    bool InitFrameBuffer(size_t frameIndex, const WebPIterator *webpFrame) noexcept;
    void ApplyPostProcessing(size_t frameIndex, WebPIDecoder *decoder, WebPDecBuffer &decoderBuffer,
                             bool blend) noexcept;
    KRYS_NODISCARD size_t FindFirstRequiredFrameToDecode(size_t frameIndex, WebPDemuxer *demuxer) noexcept;
  };
}
