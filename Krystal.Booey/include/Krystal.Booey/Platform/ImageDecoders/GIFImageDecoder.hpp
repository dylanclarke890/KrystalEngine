#pragma once

#include "Krystal.Booey/Platform/ImageDecoders/ScalableImageDecoder.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"

namespace krys::boo
{
  class GIFImageReader;

  // This class decodes the GIF image format.
  class GIFImageDecoder final : public ScalableImageDecoder
  {
  public:
    enum GIFQuery : uint8
    {
      GIFFullQuery,
      GIFSizeQuery,
      GIFFrameCountQuery
    };

  private:
    bool _currentBufferSawAlpha {false};
    mutable RepetitionCount _repetitionCount {RepetitionCountOnce};
    UniquePtr<GIFImageReader> _reader;

  public:
    KRYS_NODISCARD static Ref<ScalableImageDecoder>
      Create(AlphaOption alphaOption, GammaAndColorProfileOption gammaAndColorProfileOption) noexcept
    {
      return AdoptRef(*new GIFImageDecoder(alphaOption, gammaAndColorProfileOption));
    }

    ~GIFImageDecoder() noexcept override;

    // ScalableImageDecoder
    KRYS_NODISCARD utf8_string FilenameExtension() const noexcept final
    {
      return u8"gif";
    }
    void SetData(const SharedBuffer &data, bool allDataReceived) noexcept final;
    bool SetSize(const IntSize &size) noexcept final;
    KRYS_NODISCARD size_t FrameCount() const noexcept final;
    KRYS_NODISCARD RepetitionCount GetRepetitionCount() const noexcept final;
    ScalableImageDecoderFrame *FrameBufferAtIndex(size_t index) noexcept final;
    // CAUTION: setFailed() deletes |m_reader|. Be careful to avoid
    // accessing deleted memory, especially when calling this from inside
    // GIFImageReader!
    bool SetFailed() noexcept final;
    void ClearFrameBufferCache(size_t clearBeforeFrame) noexcept final;

    // Callbacks from the GIF reader.
    bool HaveDecodedRow(unsigned frameIndex, const List<unsigned char> &rowBuffer, size_t width,
                        size_t rowNumber, unsigned repeatCount, bool writeTransparentPixels) noexcept;
    bool FrameComplete(unsigned frameIndex, unsigned frameDuration,
                       ScalableImageDecoderFrame::DisposalMethod disposalMethod) noexcept;
    void GifComplete() noexcept;

  private:
    GIFImageDecoder(AlphaOption, GammaAndColorProfileOption) noexcept;
    void TryDecodeSize(bool allDataReceived) noexcept final
    {
      Decode(0, GIFSizeQuery, allDataReceived);
    }
    KRYS_NODISCARD size_t FindFirstRequiredFrameToDecode(size_t frameIndex) noexcept;

    // If the query is GIFFullQuery, decodes the image up to (but not
    // including) |haltAtFrame|. Otherwise, decodes as much as is needed to
    // answer the query, ignoring bitmap data. If decoding fails but there
    // is no more data coming, sets the "decode failure" flag.
    void Decode(unsigned haltAtFrame, GIFQuery query, bool allDataReceived) noexcept;

    // Called to initialize the frame buffer with the given index, based on
    // the previous frame's disposal method. Returns true on success. On
    // failure, this will mark the image as failed.
    bool InitFrameBuffer(unsigned frameIndex) noexcept;
  };
}
