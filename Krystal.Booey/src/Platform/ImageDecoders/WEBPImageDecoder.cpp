#include "Krystal.Booey/Platform/ImageDecoders/WEBPImageDecoder.hpp"
#include <algorithm>
#include <webp/decode.h>
#include <webp/demux.h>

namespace krys::boo
{
  namespace
  {
    // Convenience function to improve code readability, as WebPDemuxGetFrame is +1 based.
    bool WebPFrameAtIndex(WebPDemuxer *demuxer, size_t index, WebPIterator *webpFrame) noexcept
    {
      return WebPDemuxGetFrame(demuxer, static_cast<int>(index + 1), webpFrame);
    }
  }

  WEBPImageDecoder::WEBPImageDecoder(AlphaOption alphaOption,
                                     GammaAndColorProfileOption gammaAndColorProfileOption) noexcept
      : ScalableImageDecoder(alphaOption, gammaAndColorProfileOption)
  {
  }

  WEBPImageDecoder::~WEBPImageDecoder() noexcept = default;

  void WEBPImageDecoder::SetData(const SharedBuffer &data, bool allDataReceived) noexcept
  {
    if (Failed())
    {
      return;
    }

    // We need to ensure that the header is parsed everytime new data arrives, as the number
    // of frames may change until we have the complete data. If the size has not been obtained
    // yet, the call to ScalableImageDecoder::setData() will call parseHeader() and set
    // m_headerParsed to true, so the call to parseHeader() at the end won't do anything. If the size
    // is available, then parseHeader() is only called once.
    _headerParsed = false;
    ScalableImageDecoder::SetData(data, allDataReceived);
    ParseHeader();
  }

  RepetitionCount WEBPImageDecoder::GetRepetitionCount() const noexcept
  {
    if (Failed())
    {
      return RepetitionCountOnce;
    }

    return _repetitionCount ? _repetitionCount : RepetitionCountInfinite;
  }

  ScalableImageDecoderFrame *WEBPImageDecoder::FrameBufferAtIndex(size_t index) noexcept
  {
    if (index >= FrameCount())
    {
      return nullptr;
    }

    // The size of m_frameBufferCache may be smaller than the index requested. This can happen
    // because new data may have arrived with a bigger frameCount, but decode() hasn't been called
    // yet, which is the one that resizes the cache.
    if ((_frameBufferCache.size() > index) && _frameBufferCache[index].IsComplete())
    {
      return &_frameBufferCache[index];
    }

    Decode(index, IsAllDataReceived());

    if (index >= _frameBufferCache.size())
    {
      return nullptr;
    }
    return &_frameBufferCache[index];
  }

  size_t WEBPImageDecoder::FindFirstRequiredFrameToDecode(size_t frameIndex, WebPDemuxer *demuxer) noexcept
  {
    // The first frame doesn't depend on any other.
    if (!frameIndex)
    {
      return 0;
    }

    // Go backwards and find the first complete frame.
    size_t firstIncompleteFrame = frameIndex;
    for (; firstIncompleteFrame; --firstIncompleteFrame)
    {
      if (_frameBufferCache[firstIncompleteFrame - 1].IsComplete())
      {
        break;
      }
    }

    // Check if there are any independent frames between firstIncompleteFrame and frameIndex.
    for (size_t firstIndependentFrame = frameIndex; firstIndependentFrame > firstIncompleteFrame;
         --firstIndependentFrame)
    {
      WebPIterator webpFrame;
      if (!WebPFrameAtIndex(demuxer, firstIndependentFrame, &webpFrame))
      {
        continue;
      }

      IntRect frameRect(webpFrame.x_offset, webpFrame.y_offset, webpFrame.width, webpFrame.height);
      if (!frameRect.Contains(IntRect(IntPoint(), Size())))
      {
        continue;
      }

      // This frame covers the whole area and doesn't have alpha, so it can be rendered without
      // dependencies.
      if (!webpFrame.has_alpha)
      {
        return firstIndependentFrame;
      }

      // This frame covers the whole area and its disposalMethod is RestoreToBackground, which means
      // that the next frame will be rendered on top of a transparent background, and can be decoded
      // without dependencies. This can only be checked for frames prior to frameIndex.
      if (firstIndependentFrame < frameIndex
          && _frameBufferCache[firstIndependentFrame].GetDisposalMethod()
               == ScalableImageDecoderFrame::DisposalMethod::RestoreToBackground)
      {
        return firstIndependentFrame + 1;
      }
    }

    return firstIncompleteFrame;
  }

  void WEBPImageDecoder::Decode(size_t frameIndex, bool allDataReceived) noexcept
  {
    if (Failed())
    {
      return;
    }

    RefPtr<const SharedBuffer> protectedData = _data;
    WebPData inputData {protectedData->Data(), protectedData->Size()};
    WebPDemuxState demuxerState;
    WebPDemuxer *demuxer = WebPDemuxPartial(&inputData, &demuxerState);
    if (!demuxer)
    {
      (void)SetFailed();
      return;
    }

    _frameBufferCache.resize(_frameCount);

    // It is a fatal error if all data is received and we have decoded all frames available but the file is
    // truncated.
    if (frameIndex >= _frameBufferCache.size() - 1 && allDataReceived && demuxerState != WEBP_DEMUX_DONE)
    {
      WebPDemuxDelete(demuxer);
      (void)SetFailed();
      return;
    }

    for (size_t i = FindFirstRequiredFrameToDecode(frameIndex, demuxer); i <= frameIndex; i++)
    {
      DecodeFrame(i, demuxer);
    }

    WebPDemuxDelete(demuxer);
  }

  void WEBPImageDecoder::DecodeFrame(size_t frameIndex, WebPDemuxer *demuxer) noexcept
  {
    if (Failed())
    {
      return;
    }

    WebPIterator webpFrame;
    if (!WebPFrameAtIndex(demuxer, frameIndex, &webpFrame))
    {
      return;
    }

    const uint8 *dataBytes = webpFrame.fragment.bytes;
    size_t dataSize = webpFrame.fragment.size;
    bool blend = webpFrame.blend_method == WEBP_MUX_BLEND;

    krys_debug_assert(_frameBufferCache.size() > frameIndex);
    auto &buffer = _frameBufferCache[frameIndex];
    buffer.SetDuration(milliseconds(webpFrame.duration));
    buffer.SetDisposalMethod(webpFrame.dispose_method == WEBP_MUX_DISPOSE_BACKGROUND
                               ? ScalableImageDecoderFrame::DisposalMethod::RestoreToBackground
                               : ScalableImageDecoderFrame::DisposalMethod::DoNotDispose);
    krys_debug_assert(!buffer.IsComplete());

    if (buffer.IsInvalid() && !InitFrameBuffer(frameIndex, &webpFrame))
    {
      (void)SetFailed();
      return;
    }

    WebPDecBuffer decoderBuffer;
    WebPInitDecBuffer(&decoderBuffer);
    decoderBuffer.colorspace = MODE_RGBA;
    decoderBuffer.u.RGBA.stride = webpFrame.width * static_cast<int>(sizeof(uint32));
    decoderBuffer.u.RGBA.size = static_cast<size_t>(decoderBuffer.u.RGBA.stride) * webpFrame.height;
    decoderBuffer.is_external_memory = 1;
    auto pixels = UniquePtr<uint8[]>(new uint8[decoderBuffer.u.RGBA.size]);
    decoderBuffer.u.RGBA.rgba = pixels.get();

    WebPIDecoder *decoder = WebPINewDecoder(&decoderBuffer);
    if (!decoder)
    {
      (void)SetFailed();
      return;
    }

    switch (WebPIUpdate(decoder, dataBytes, dataSize))
    {
      case VP8_STATUS_OK:
        ApplyPostProcessing(frameIndex, decoder, decoderBuffer, blend);
        buffer.SetDecodingStatus(DecodingStatus::Complete);
        break;
      case VP8_STATUS_SUSPENDED:
        if (!IsAllDataReceived())
        {
          ApplyPostProcessing(frameIndex, decoder, decoderBuffer, blend);
          buffer.SetDecodingStatus(DecodingStatus::Partial);
          break;
        }
        [[fallthrough]];
      default: (void)SetFailed();
    }

    WebPIDelete(decoder);
  }

  bool WEBPImageDecoder::InitFrameBuffer(size_t frameIndex, const WebPIterator *webpFrame) noexcept
  {
    if (frameIndex >= FrameCount())
    {
      return false;
    }

    auto &buffer = _frameBufferCache[frameIndex];

    // Initialize the frame rect in our buffer.
    IntRect frameRect(webpFrame->x_offset, webpFrame->y_offset, webpFrame->width, webpFrame->height);

    // Make sure the frameRect doesn't extend outside the buffer.
    frameRect.Intersect(IntRect(IntPoint(), Size()));

    if (!frameIndex || !_frameBufferCache[frameIndex - 1].BackingStore())
    {
      // This frame doesn't rely on any previous data.
      if (!buffer.Initialize(Size(), _premultiplyAlpha))
      {
        return false;
      }
    }
    else
    {
      const auto &prevBuffer = _frameBufferCache[frameIndex - 1];
      krys_debug_assert(prevBuffer.IsComplete());

      // Preserve the last frame as the starting state for this frame.
      if (!prevBuffer.BackingStore() || !buffer.Initialize(*prevBuffer.BackingStore()))
      {
        return false;
      }

      if (prevBuffer.GetDisposalMethod() == ScalableImageDecoderFrame::DisposalMethod::RestoreToBackground)
      {
        // We want to clear the previous frame to transparent, without
        // affecting pixels in the image outside of the frame.
        const IntRect &prevRect = prevBuffer.BackingStore()->FrameRect();
        buffer.BackingStore()->ClearRect(prevRect);
      }
    }

    buffer.SetHasAlpha(webpFrame->has_alpha);
    buffer.BackingStore()->SetFrameRect(frameRect);

    return true;
  }

  void WEBPImageDecoder::ApplyPostProcessing(size_t frameIndex, WebPIDecoder *decoder,
                                             WebPDecBuffer &decoderBuffer, bool blend) noexcept
  {
    auto &buffer = _frameBufferCache[frameIndex];
    int decodedWidth = 0;
    int decodedHeight = 0;
    if (!WebPIDecGetRGB(decoder, &decodedHeight, &decodedWidth, nullptr, nullptr))
    {
      return; // See also https://bugs.webkit.org/show_bug.cgi?id=74062
    }
    if (decodedHeight <= 0)
    {
      return;
    }

    const IntRect &frameRect = buffer.BackingStore()->FrameRect();
    krys_assert(decodedWidth == frameRect.Width());
    krys_assert(decodedHeight <= frameRect.Height());
    const int left = frameRect.X();
    const int top = frameRect.Y();

    for (int y = 0; y < decodedHeight; y++)
    {
      const int canvasY = top + y;
      for (int x = 0; x < decodedWidth; x++)
      {
        const int canvasX = left + x;
        auto &destinationPixel = buffer.BackingStore()->PixelAt(canvasX, canvasY);
        uint8 *sourcePixels =
          decoderBuffer.u.RGBA.rgba + (static_cast<size_t>(y) * frameRect.Width() + x) * sizeof(uint32);
        if (blend && (sourcePixels[3] < 255))
        {
          buffer.BackingStore()->BlendPixel(destinationPixel, sourcePixels[0], sourcePixels[1],
                                            sourcePixels[2], sourcePixels[3]);
        }
        else
        {
          buffer.BackingStore()->SetPixel(destinationPixel, sourcePixels[0], sourcePixels[1], sourcePixels[2],
                                          sourcePixels[3]);
        }
      }
    }
  }

  void WEBPImageDecoder::ParseHeader() noexcept
  {
    if (_headerParsed)
    {
      return;
    }

    _headerParsed = true;

    constexpr size_t webpHeaderSize = 30; // RIFF_HEADER_SIZE + CHUNK_HEADER_SIZE + VP8_FRAME_HEADER_SIZE
    if (_data->Size() < webpHeaderSize)
    {
      return; // Await VP8X header so WebPDemuxPartial succeeds.
    }

    WebPData inputData {_data->Data(), _data->Size()};
    WebPDemuxState demuxerState;
    WebPDemuxer *demuxer = WebPDemuxPartial(&inputData, &demuxerState);
    if (!demuxer)
    {
      (void)SetFailed();
      return;
    }

    _frameCount = WebPDemuxGetI(demuxer, WEBP_FF_FRAME_COUNT);
    if (!_frameCount)
    {
      WebPDemuxDelete(demuxer);
      return; // Wait until the encoded image frame data arrives.
    }

    int width = static_cast<int>(WebPDemuxGetI(demuxer, WEBP_FF_CANVAS_WIDTH));
    int height = static_cast<int>(WebPDemuxGetI(demuxer, WEBP_FF_CANVAS_HEIGHT));
    if (!IsSizeAvailable() && !SetSize(IntSize(width, height)))
    {
      WebPDemuxDelete(demuxer);
      return;
    }

    _formatFlags = static_cast<int32>(WebPDemuxGetI(demuxer, WEBP_FF_FORMAT_FLAGS));
    if (!(_formatFlags & ANIMATION_FLAG))
    {
      _repetitionCount = RepetitionCountNone;
    }
    else
    {
      // Since we have parsed at least one frame, even if partially,
      // the global animation (ANIM) properties have been read since
      // an ANIM chunk must precede the ANMF frame chunks.
      _repetitionCount = static_cast<int32>(WebPDemuxGetI(demuxer, WEBP_FF_LOOP_COUNT));
      krys_debug_assert(_repetitionCount == (_repetitionCount & 0xffff)); // Loop count is always <= 16 bits.
      if (!_repetitionCount)
      {
        _repetitionCount = RepetitionCountInfinite;
      }
    }

    WebPDemuxDelete(demuxer);
  }

  void WEBPImageDecoder::ClearFrameBufferCache(size_t clearBeforeFrame) noexcept
  {
    if (_frameBufferCache.empty())
    {
      return;
    }

    // We don't want to delete the last frame in the cache, as is may be needed for
    // decoding when new data arrives. See GIFImageDecoder for the full explanation.
    clearBeforeFrame = std::min(clearBeforeFrame, _frameBufferCache.size() - 1);

    // Also from GIFImageDecoder: We need to preserve frames such that:
    //   * We don't clear |clearBeforeFrame|.
    //   * We don't clear the frame we're currently decoding.
    //   * We don't clear any frame from which a future initFrameBuffer() call will copy bitmap data.
    //
    // In WEBP every frame depends on the previous one or none. That means that frames after clearBeforeFrame
    // won't need any frame before them to render, so we can clear them all.
    bool skipNextComplete = true;
    for (int64 i = static_cast<int64>(clearBeforeFrame) - 1; i >= 0; i--)
    {
      auto &buffer = _frameBufferCache[static_cast<size_t>(i)];
      if (skipNextComplete && buffer.IsComplete())
      {
        skipNextComplete = false;
        continue;
      }
      if (!buffer.IsInvalid())
      {
        buffer.Clear();
      }
    }
  }
}
