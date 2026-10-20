#include "Krystal.Booey/Platform/ImageDecoders/GIFImageDecoder.hpp"
#include "Krystal.Booey/Platform/ImageDecoders/GIFImageReader.hpp"
#include <algorithm>
#include <limits>

namespace krys::boo
{
  GIFImageDecoder::GIFImageDecoder(AlphaOption alphaOption,
                                   GammaAndColorProfileOption gammaAndColorProfileOption) noexcept
      : ScalableImageDecoder(alphaOption, gammaAndColorProfileOption)
  {
  }

  GIFImageDecoder::~GIFImageDecoder() noexcept = default;

  void GIFImageDecoder::SetData(const SharedBuffer &data, bool allDataReceived) noexcept
  {
    if (Failed())
    {
      return;
    }

    ScalableImageDecoder::SetData(data, allDataReceived);
    if (_reader)
    {
      _reader->SetData(ShareRef(*_data));
    }
  }

  bool GIFImageDecoder::SetSize(const IntSize &size) noexcept
  {
    if (ScalableImageDecoder::GetEncodedDataStatus() >= EncodedDataStatus::SizeAvailable && Size() == size)
    {
      return true;
    }

    return ScalableImageDecoder::SetSize(size);
  }

  size_t GIFImageDecoder::FrameCount() const noexcept
  {
    const_cast<GIFImageDecoder *>(this)->Decode(std::numeric_limits<unsigned>::max(), GIFFrameCountQuery,
                                                IsAllDataReceived());
    return _frameBufferCache.size();
  }

  RepetitionCount GIFImageDecoder::GetRepetitionCount() const noexcept
  {
    // This value can arrive at any point in the image data stream.  Most GIFs
    // in the wild declare it near the beginning of the file, so it usually is
    // set by the time we've decoded the size, but (depending on the GIF and the
    // packets sent back by the webserver) not always.  If the reader hasn't
    // seen a loop count yet, it will return cLoopCountNotSeen, in which case we
    // should default to looping once (the initial value for
    // |m_repetitionCount|).
    //
    // There are some additional wrinkles here. First, ImageSource::clear()
    // may destroy the reader, making the result from the reader _less_
    // authoritative on future calls if the recreated reader hasn't seen the
    // loop count.  We don't need to special-case this because in this case the
    // new reader will once again return cLoopCountNotSeen, and we won't
    // overwrite the cached correct value.
    //
    // Second, a GIF might never set a loop count at all, in which case we
    // should continue to treat it as a "loop once" animation.  We don't need
    // special code here either, because in this case we'll never change
    // |m_repetitionCount| from its default value.
    //
    // Third, we use the same GIFImageReader for counting frames and we might
    // see the loop count and then encounter a decoding error which happens
    // later in the stream. It is also possible that no frames are in the
    // stream. In these cases we should just loop once.
    if (Failed() || (_reader && (!_reader->ImagesCount())))
    {
      _repetitionCount = RepetitionCountOnce;
    }
    else if (_reader && _reader->LoopCount() != LoopCountNotSeen)
    {
      _repetitionCount = _reader->LoopCount() > 0 ? _reader->LoopCount() + 1 : _reader->LoopCount();
    }
    return _repetitionCount;
  }

  size_t GIFImageDecoder::FindFirstRequiredFrameToDecode(size_t frameIndex) noexcept
  {
    // The first frame doesn't depend on any other.
    if (!frameIndex)
    {
      return 0;
    }

    for (size_t i = frameIndex; i > 0; --i)
    {
      auto &frame = _frameBufferCache[i - 1];

      // Frames with disposal method RestoreToPrevious are useless, skip them.
      if (frame.GetDisposalMethod() == ScalableImageDecoderFrame::DisposalMethod::RestoreToPrevious)
      {
        continue;
      }

      // At this point the disposal method can be Unspecified, DoNotDispose or RestoreToBackground.
      // In every case, if the frame is complete we can start decoding the next one.
      if (frame.IsComplete())
      {
        return i;
      }

      // If the disposal method of this frame is RestoreToBackground and it fills the whole area,
      // the next frame's backing store is initialized to transparent, so we start decoding with it.
      if (frame.GetDisposalMethod() == ScalableImageDecoderFrame::DisposalMethod::RestoreToBackground)
      {
        // We cannot use frame.backingStore()->frameRect() here, because it has been cleared
        // when the frame was removed from the cache. We need to get the values from the
        // reader context.
        const auto *frameContext = _reader->FrameContext(i - 1);
        krys_debug_assert(frameContext);
        IntRect frameRect(static_cast<int32>(frameContext->XOffset),
                          static_cast<int32>(frameContext->YOffset), static_cast<int32>(frameContext->Width),
                          static_cast<int32>(frameContext->Height));
        if (frameRect.Contains(IntRect(IntPoint(), Size())))
        {
          return i;
        }
      }
    }

    return 0;
  }

  ScalableImageDecoderFrame *GIFImageDecoder::FrameBufferAtIndex(size_t index) noexcept
  {
    if (index >= FrameCount())
    {
      return nullptr;
    }

    auto &frame = _frameBufferCache[index];
    if (!frame.IsComplete())
    {
      for (auto i = FindFirstRequiredFrameToDecode(index); i <= index; i++)
      {
        Decode(static_cast<unsigned>(i + 1), GIFFullQuery, IsAllDataReceived());
      }
    }

    return index < _frameBufferCache.size() ? &_frameBufferCache[index] : nullptr;
  }

  bool GIFImageDecoder::SetFailed() noexcept
  {
    _reader = nullptr;
    return ScalableImageDecoder::SetFailed();
  }

  void GIFImageDecoder::ClearFrameBufferCache(size_t clearBeforeFrame) noexcept
  {
    // In some cases, like if the decoder was destroyed while animating, we
    // can be asked to clear more frames than we currently have.
    if (_frameBufferCache.empty())
    {
      return; // Nothing to do.
    }

    // The "-1" here is tricky.  It does not mean that |clearBeforeFrame| is the
    // last frame we wish to preserve, but rather that we never want to clear
    // the very last frame in the cache: it's empty (so clearing it is
    // pointless), it's partial (so we don't want to clear it anyway), or the
    // cache could be enlarged with a future setData() call and it could be
    // needed to construct the next frame (see comments below).  Callers can
    // always use ImageSource::clear(true, ...) to completely free the memory in
    // this case.
    clearBeforeFrame = std::min(clearBeforeFrame, _frameBufferCache.size() - 1);
    const auto end = _frameBufferCache.begin() + static_cast<ptrdiff_t>(clearBeforeFrame);

    // We need to preserve frames such that:
    //   * We don't clear |end|
    //   * We don't clear the frame we're currently decoding
    //   * We don't clear any frame from which a future initFrameBuffer() call
    //     will copy bitmap data
    // All other frames can be cleared.  Because of the constraints on when
    // ImageSource::clear() can be called (see ImageSource.h), we're guaranteed
    // not to have non-empty frames after the frame we're currently decoding.
    // So, scan backwards from |end| as follows:
    //   * If the frame is empty, we're still past any frames we care about.
    //   * If the frame is complete, but is DisposalMethod::RestoreToPrevious, we'll
    //     skip over it in future initFrameBuffer() calls.  We can clear it
    //     unless it's |end|, and keep scanning.  For any other disposal method,
    //     stop scanning, as we've found the frame initFrameBuffer() will need
    //     next.
    //   * If the frame is partial, we're decoding it, so don't clear it; if it
    //     has a disposal method other than DisposalMethod::RestoreToPrevious, stop
    //     scanning, as we'll only need this frame when decoding the next one.
    auto i = end;
    for (; (i != _frameBufferCache.begin())
           && (i->IsInvalid()
               || (i->GetDisposalMethod() == ScalableImageDecoderFrame::DisposalMethod::RestoreToPrevious));
         --i)
    {
      if (i->IsComplete() && (i != end))
      {
        i->Clear();
      }
    }

    // Now |i| holds the last frame we need to preserve; clear prior frames.
    for (auto j = _frameBufferCache.begin(); j != i; ++j)
    {
      krys_debug_assert(!j->IsPartial());
      if (!j->IsInvalid())
      {
        j->Clear();
      }
    }
  }

  bool GIFImageDecoder::HaveDecodedRow(unsigned frameIndex, const List<unsigned char> &rowBuffer,
                                       size_t width, size_t rowNumber, unsigned repeatCount,
                                       bool writeTransparentPixels) noexcept
  {
    const GIFFrameContext *frameContext = _reader->FrameContext();
    // The pixel data and coordinates supplied to us are relative to the frame's
    // origin within the entire image size, i.e.
    // (frameContext->xOffset, frameContext->yOffset). There is no guarantee
    // that width == (size().width() - frameContext->xOffset), so
    // we must ensure we don't run off the end of either the source data or the
    // row's X-coordinates.
    int xBegin = static_cast<int>(frameContext->XOffset);
    int yBegin = static_cast<int>(frameContext->YOffset + rowNumber);
    int xEnd = std::min(static_cast<int>(frameContext->XOffset + width), Size().Width());
    int yEnd = std::min(static_cast<int>(frameContext->YOffset + rowNumber + repeatCount), Size().Height());
    if (rowBuffer.empty() || xEnd <= xBegin || yEnd <= yBegin)
    {
      return true;
    }

    // Get the colormap.
    Span<const uint8> colorMap;
    if (frameContext->IsLocalColormapDefined)
    {
      colorMap = _reader->LocalColormap(frameContext);
    }
    else
    {
      colorMap = _reader->GlobalColormap();
    }
    if (colorMap.empty())
    {
      return true;
    }

    // Initialize the frame if necessary.
    auto &buffer = _frameBufferCache[frameIndex];
    if ((buffer.IsInvalid() && !InitFrameBuffer(frameIndex)) || !buffer.HasBackingStore())
    {
      return false;
    }

    auto *currentAddress = buffer.BackingStore()->PixelsStartingAt(xBegin, yBegin);
    // Write one row's worth of data into the frame.
    for (int x = xBegin; x < xEnd; ++x, ++currentAddress)
    {
      const unsigned char sourceValue = rowBuffer[static_cast<size_t>(x) - frameContext->XOffset];
      const size_t colorIndex = static_cast<size_t>(sourceValue) * 3;
      if ((!frameContext->IsTransparent || (sourceValue != frameContext->Tpixel))
          && (colorIndex + 2 < colorMap.size()))
      {
        buffer.BackingStore()->SetPixel(*currentAddress, colorMap[colorIndex], colorMap[colorIndex + 1],
                                        colorMap[colorIndex + 2], 255);
      }
      else
      {
        _currentBufferSawAlpha = true;
        // We may or may not need to write transparent pixels to the buffer.
        // If we're compositing against a previous image, it's wrong, and if
        // we're writing atop a cleared, fully transparent buffer, it's
        // unnecessary; but if we're decoding an interlaced gif and
        // displaying it "Haeberli"-style, we must write these for passes
        // beyond the first, or the initial passes will "show through" the
        // later ones.
        if (writeTransparentPixels)
        {
          buffer.BackingStore()->SetPixel(*currentAddress, 0, 0, 0, 0);
        }
      }
    }

    // Tell the frame to copy the row data if need be.
    if (repeatCount > 1)
    {
      buffer.BackingStore()->RepeatFirstRow(IntRect(xBegin, yBegin, xEnd - xBegin, yEnd - yBegin));
    }

    return true;
  }

  bool GIFImageDecoder::FrameComplete(unsigned frameIndex, unsigned frameDuration,
                                      ScalableImageDecoderFrame::DisposalMethod disposalMethod) noexcept
  {
    // Initialize the frame if necessary.  Some GIFs insert do-nothing frames,
    // in which case we never reach haveDecodedRow() before getting here.
    auto &buffer = _frameBufferCache[frameIndex];
    if (buffer.IsInvalid() && !InitFrameBuffer(frameIndex))
    {
      return false; // initFrameBuffer() has already called setFailed().
    }

    buffer.SetDecodingStatus(DecodingStatus::Complete);
    buffer.SetDuration(milliseconds(frameDuration));
    buffer.SetDisposalMethod(disposalMethod);

    if (!_currentBufferSawAlpha)
    {
      IntRect rect = buffer.BackingStore()->FrameRect();

      // The whole frame was non-transparent, so it's possible that the entire
      // resulting buffer was non-transparent, and we can setHasAlpha(false).
      if (rect.Contains(IntRect(IntPoint(), Size())))
      {
        buffer.SetHasAlpha(false);
      }
      else if (frameIndex)
      {
        // Tricky case.  This frame does not have alpha only if everywhere
        // outside its rect doesn't have alpha.  To know whether this is
        // true, we check the start state of the frame -- if it doesn't have
        // alpha, we're safe.
        //
        // First skip over prior DisposalMethod::RestoreToPrevious frames (since they
        // don't affect the start state of this frame) the same way we do in
        // initFrameBuffer().
        const auto *prevBuffer = &_frameBufferCache[--frameIndex];
        while (frameIndex
               && (prevBuffer->GetDisposalMethod()
                   == ScalableImageDecoderFrame::DisposalMethod::RestoreToPrevious))
        {
          prevBuffer = &_frameBufferCache[--frameIndex];
        }

        // Now, if we're at a DisposalMethod::Unspecified or DisposalMethod::DoNotDispose frame, then
        // we can say we have no alpha if that frame had no alpha.  But
        // since in initFrameBuffer() we already copied that frame's alpha
        // state into the current frame's, we need do nothing at all here.
        //
        // The only remaining case is a DisposalMethod::RestoreToBackground frame. If
        // it had no alpha, and its rect is contained in the current frame's
        // rect, we know the current frame has no alpha.
        IntRect prevRect = prevBuffer->BackingStore()->FrameRect();
        if ((prevBuffer->GetDisposalMethod()
             == ScalableImageDecoderFrame::DisposalMethod::RestoreToBackground)
            && !prevBuffer->HasAlpha() && rect.Contains(prevRect))
        {
          buffer.SetHasAlpha(false);
        }
      }
    }

    return true;
  }

  void GIFImageDecoder::GifComplete() noexcept
  {
    // Cache the repetition count, which is now as authoritative as it's ever
    // going to be.
    (void)GetRepetitionCount();

    _reader = nullptr;
  }

  void GIFImageDecoder::Decode(unsigned haltAtFrame, GIFQuery query, bool allDataReceived) noexcept
  {
    if (Failed())
    {
      return;
    }

    if (!_reader)
    {
      _reader = CreateUnique<GIFImageReader>(this);
      _reader->SetData(ShareRef(*_data));
    }

    if (query == GIFSizeQuery)
    {
      if (!_reader->Decode(GIFSizeQuery, haltAtFrame))
      {
        (void)SetFailed();
      }
      return;
    }

    if (!_reader->Decode(GIFFrameCountQuery, haltAtFrame))
    {
      (void)SetFailed();
      return;
    }

    _frameBufferCache.resize(_reader->ImagesCount());

    if (query == GIFFrameCountQuery)
    {
      return;
    }

    if (!_reader->Decode(GIFFullQuery, haltAtFrame))
    {
      (void)SetFailed();
      return;
    }

    // It is also a fatal error if all data is received but we failed to decode
    // all frames completely.
    if (allDataReceived && haltAtFrame >= _frameBufferCache.size() && _reader)
    {
      (void)SetFailed();
    }
  }

  bool GIFImageDecoder::InitFrameBuffer(unsigned frameIndex) noexcept
  {
    // Initialize the frame rect in our buffer.
    const GIFFrameContext *frameContext = _reader->FrameContext();
    IntRect frameRect(static_cast<int32>(frameContext->XOffset), static_cast<int32>(frameContext->YOffset),
                      static_cast<int32>(frameContext->Width), static_cast<int32>(frameContext->Height));
    auto *const buffer = &_frameBufferCache[frameIndex];

    if (!frameIndex)
    {
      // This is the first frame, so we're not relying on any previous data.
      if (!buffer->Initialize(Size(), _premultiplyAlpha))
      {
        return SetFailed();
      }
    }
    else
    {
      // The starting state for this frame depends on the previous frame's
      // disposal method.
      //
      // Frames that use the DisposalMethod::RestoreToPrevious method are effectively
      // no-ops in terms of changing the starting state of a frame compared to
      // the starting state of the previous frame, so skip over them.  (If the
      // first frame specifies this method, it will get treated like
      // DisposalMethod::RestoreToBackground below and reset to a completely empty image.)
      const auto *prevBuffer = &_frameBufferCache[--frameIndex];
      auto prevMethod = prevBuffer->GetDisposalMethod();
      while (frameIndex && (prevMethod == ScalableImageDecoderFrame::DisposalMethod::RestoreToPrevious))
      {
        prevBuffer = &_frameBufferCache[--frameIndex];
        prevMethod = prevBuffer->GetDisposalMethod();
      }

      krys_debug_assert(prevBuffer->IsComplete());

      if ((prevMethod == ScalableImageDecoderFrame::DisposalMethod::Unspecified)
          || (prevMethod == ScalableImageDecoderFrame::DisposalMethod::DoNotDispose))
      {
        // Preserve the last frame as the starting state for this frame.
        if (!prevBuffer->BackingStore() || !buffer->Initialize(*prevBuffer->BackingStore()))
        {
          return SetFailed();
        }
      }
      else
      {
        // We want to clear the previous frame to transparent, without
        // affecting pixels in the image outside of the frame.
        IntRect prevRect = prevBuffer->BackingStore()->FrameRect();
        const IntSize bufferSize = Size();
        if (!frameIndex || prevRect.Contains(IntRect(IntPoint(), Size())))
        {
          // Clearing the first frame, or a frame the size of the whole
          // image, results in a completely empty image.
          if (!buffer->Initialize(bufferSize, _premultiplyAlpha))
          {
            return SetFailed();
          }
        }
        else
        {
          // Copy the whole previous buffer, then clear just its frame.
          if (!prevBuffer->BackingStore() || !buffer->Initialize(*prevBuffer->BackingStore()))
          {
            return SetFailed();
          }
          buffer->BackingStore()->ClearRect(prevRect);
          buffer->SetHasAlpha(true);
        }
      }
    }

    // Make sure the frameRect doesn't extend outside the buffer.
    if (frameRect.MaxX() > Size().Width())
    {
      frameRect.SetWidth(Size().Width() - static_cast<int32>(frameContext->XOffset));
    }
    if (frameRect.MaxY() > Size().Height())
    {
      frameRect.SetHeight(Size().Height() - static_cast<int32>(frameContext->YOffset));
    }

    buffer->BackingStore()->SetFrameRect(frameRect);

    // Update our status to be partially complete.
    buffer->SetDecodingStatus(DecodingStatus::Partial);

    // Reset the alpha pixel tracker for this frame.
    _currentBufferSawAlpha = false;
    return true;
  }
}
