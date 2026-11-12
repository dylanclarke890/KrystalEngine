/*
 * Copyright (C) 2024-2025 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/Graphics/Images/BitmapImageSource.hpp"
#include "Krystal.Booey/Graphics/Images/BitmapImage.hpp"
#include "Krystal.Booey/Graphics/Images/ImageDecoder.hpp"
#include "Krystal.Booey/Graphics/Images/ImageObserver.hpp"
#include "Krystal.Booey/Platform/SharedBuffer.hpp"
#include "Krystal.Core/Text/TextStream.hpp"
#include <cmath>
#include <limits>

namespace krys::boo
{
  namespace
  {
    /// @brief @p image's pixels laid out as drawn with @p orientation, at @p size (the oriented size).
    PlatformImagePtr OrientedPlatformImage(const PlatformImage &image, ImageOrientation orientation,
                                           const IntSize &size) noexcept
    {
      const auto &sourceSize = image.Size();
      const auto transform = orientation.TransformFromDefault(FloatSize(size));
      auto pixels = AdoptRef(*new PixelData(static_cast<size_t>(size.UnclampedArea())));
      const auto source = image.Pixels();
      for (int32 y = 0; y < sourceSize.Height(); ++y)
      {
        for (int32 x = 0; x < sourceSize.Width(); ++x)
        {
          // Map the pixel's centre; every orientation maps whole pixels onto whole pixels.
          const auto mapped =
            transform.MapPoint(FloatPoint(static_cast<float>(x) + 0.5f, static_cast<float>(y) + 0.5f));
          const auto destX = static_cast<int32>(std::floor(mapped.X()));
          const auto destY = static_cast<int32>(std::floor(mapped.Y()));
          if (destX < 0 || destY < 0 || destX >= size.Width() || destY >= size.Height())
          {
            continue;
          }
          pixels->Pixels[static_cast<size_t>(destY) * static_cast<size_t>(size.Width())
                         + static_cast<size_t>(destX)] =
            source[static_cast<size_t>(y) * static_cast<size_t>(sourceSize.Width()) + static_cast<size_t>(x)];
        }
      }
      return AdoptRef(*new PlatformImage(size, krys::move(pixels), image.PremultipliedAlpha()));
    }

    /// @brief @p image's pixels resampled to @p size, each the average of the area of the source it covers.
    /// @note Unpremultiplied colours are weighted by their alpha, so transparent pixels add no colour.
    PlatformImagePtr ScaledPlatformImage(const PlatformImage &image, const IntSize &size) noexcept
    {
      const auto &sourceSize = image.Size();
      const auto source = image.Pixels();
      const bool premultiplied = image.PremultipliedAlpha();
      const double scaleX = static_cast<double>(sourceSize.Width()) / size.Width();
      const double scaleY = static_cast<double>(sourceSize.Height()) / size.Height();
      auto pixels = AdoptRef(*new PixelData(static_cast<size_t>(size.UnclampedArea())));

      for (int32 y = 0; y < size.Height(); ++y)
      {
        const double top = y * scaleY;
        const double bottom = (y + 1) * scaleY;
        for (int32 x = 0; x < size.Width(); ++x)
        {
          const double left = x * scaleX;
          const double right = (x + 1) * scaleX;
          double area = 0;
          double colourWeight = 0;
          double sums[4] {};
          for (auto sourceY = static_cast<int32>(top);
               sourceY < std::min(static_cast<int32>(std::ceil(bottom)), sourceSize.Height()); ++sourceY)
          {
            const double height =
              std::min(bottom, sourceY + 1.0) - std::max(top, static_cast<double>(sourceY));
            for (auto sourceX = static_cast<int32>(left);
                 sourceX < std::min(static_cast<int32>(std::ceil(right)), sourceSize.Width()); ++sourceX)
            {
              const double weight =
                height * (std::min(right, sourceX + 1.0) - std::max(left, static_cast<double>(sourceX)));
              const auto pixel = source[static_cast<size_t>(sourceY) * static_cast<size_t>(sourceSize.Width())
                                        + static_cast<size_t>(sourceX)];
              const double alpha = (pixel >> 24) & 0xFF;
              const double weightOfColour = premultiplied ? weight : weight * alpha / 255;
              area += weight;
              colourWeight += weightOfColour;
              sums[0] += alpha * weight;
              sums[1] += ((pixel >> 16) & 0xFF) * weightOfColour;
              sums[2] += ((pixel >> 8) & 0xFF) * weightOfColour;
              sums[3] += (pixel & 0xFF) * weightOfColour;
            }
          }

          uint32 result = 0;
          for (size_t channel = 0; channel < 4; ++channel)
          {
            const double divisor = channel ? colourWeight : area;
            const auto value = divisor > 0 ? std::lround(sums[channel] / divisor) : 0l;
            result = (result << 8) | static_cast<uint32>(std::clamp(value, 0l, 255l));
          }
          pixels
            ->Pixels[static_cast<size_t>(y) * static_cast<size_t>(size.Width()) + static_cast<size_t>(x)] =
            result;
        }
      }
      return AdoptRef(*new PlatformImage(size, krys::move(pixels), premultiplied));
    }
  }

  Ref<BitmapImageSource>
    BitmapImageSource::Create(BitmapImage &bitmapImage, AlphaOption alphaOption,
                              GammaAndColorProfileOption gammaAndColorProfileOption) noexcept
  {
    return AdoptRef(*new BitmapImageSource(bitmapImage, alphaOption, gammaAndColorProfileOption));
  }

  BitmapImageSource::BitmapImageSource(BitmapImage &bitmapImage, AlphaOption alphaOption,
                                       GammaAndColorProfileOption gammaAndColorProfileOption) noexcept
      : _bitmapImage(CreateWeakPtr(&bitmapImage)), _alphaOption(alphaOption),
        _gammaAndColorProfileOption(gammaAndColorProfileOption), _descriptor(*this)
  {
  }

  BitmapImageSource::~BitmapImageSource() noexcept
  {
    // A frame still being decoded is dropped when it arrives.
    if (_workQueue)
    {
      _workQueue->Detach();
    }
  }

  ImageFrameWorkQueue &BitmapImageSource::FrameWorkQueue() const noexcept
  {
    if (!_workQueue)
    {
      _workQueue = ImageFrameWorkQueue::Create(const_cast<BitmapImageSource &>(*this));
    }
    return *_workQueue;
  }

  bool BitmapImageSource::CanDecodeAsynchronously() const noexcept
  {
    return _allDataReceived && _bitmapImage && _bitmapImage->Data() && ImageFrameWorkQueue::HostWorkQueue();
  }

  RefPtr<ImageDecoder> BitmapImageSource::CreateWorkQueueDecoder() const noexcept
  {
    auto *data = _bitmapImage ? _bitmapImage->Data() : nullptr;
    if (!data || !_allDataReceived)
    {
      return nullptr;
    }
    auto decoder = ImageDecoder::Create(*data, MimeType(), _alphaOption, _gammaAndColorProfileOption);
    if (decoder)
    {
      decoder->SetData(*data, true);
    }
    return decoder;
  }

  ImageDecoder *BitmapImageSource::Decoder(SharedBuffer *data) const noexcept
  {
    if (_decoder)
    {
      return _decoder.get();
    }

    if (!data)
    {
      return nullptr;
    }

    _decoder = ImageDecoder::Create(*data, MimeType(), _alphaOption, _gammaAndColorProfileOption);
    if (!_decoder)
    {
      return nullptr;
    }

    if (auto expectedContentLength = ExpectedContentLength())
    {
      _decoder->SetExpectedContentSize(expectedContentLength);
    }

    return _decoder.get();
  }

  void BitmapImageSource::EncodedDataStatusChanged(EncodedDataStatus status) noexcept
  {
    krys_debug_assert(_decoder);

    if (status >= EncodedDataStatus::SizeAvailable)
    {
      _frames.resize(_decoder->FrameCount());
      _frames.shrink_to_fit();
    }

    if (auto imageObserver = GetImageObserver())
    {
      imageObserver->EncodedDataStatusChanged(*_bitmapImage, status);
    }
  }

  EncodedDataStatus BitmapImageSource::DataChanged(SharedBuffer *data, bool allDataReceived) noexcept
  {
    _descriptor.Clear();

    // The work queue's decoder has the old data.
    if (_workQueue)
    {
      _workQueue->Stop();
    }

    auto status = SetData(data, allDataReceived);
    if (status < EncodedDataStatus::TypeAvailable)
    {
      return status;
    }

    EncodedDataStatusChanged(status);
    return status;
  }

  void BitmapImageSource::DestroyDecodedData(bool destroyAll) noexcept
  {
    bool canDestroyDecodedData = destroyAll && CanDestroyDecodedData();

    uint32 primaryFrameIndex = PrimaryFrameIndex();
    uint32 currentFrameIndex = CurrentFrameIndex();
    uint32 decodedSize = 0;

    for (uint32 index = 0, framesSize = static_cast<uint32>(_frames.size()); index < framesSize; ++index)
    {
      if (!canDestroyDecodedData && (index == primaryFrameIndex || index == currentFrameIndex))
      {
        continue;
      }

      if (!destroyAll && index > currentFrameIndex)
      {
        break;
      }

      decodedSize += _frames[index].ClearImage();
    }

    DecodedSizeReset(decodedSize);

    // There's no need to throw away the decoder unless we're explicitly asked
    // to destroy all of the frames.
    if (destroyAll)
    {
      ResetData();
    }
    else
    {
      ClearFrameBufferCache();
    }
  }

  void BitmapImageSource::DecodedSizeChanged(int64 decodedSize) noexcept
  {
    if (!decodedSize)
    {
      return;
    }

    if (auto imageObserver = GetImageObserver())
    {
      imageObserver->DecodedSizeChanged(*_bitmapImage, decodedSize);
    }
  }

  void BitmapImageSource::DecodedSizeIncreased(uint32 decodedSize) noexcept
  {
    if (!decodedSize)
    {
      return;
    }

    _decodedSize += decodedSize;

    // The fully-decoded frame will subsume the partially decoded data used
    // to determine image properties.
    int64 changeSize = static_cast<int64>(decodedSize) - _decodedPropertiesSize;
    _decodedPropertiesSize = 0;
    DecodedSizeChanged(changeSize);
  }

  void BitmapImageSource::DecodedSizeDecreased(uint32 decodedSize) noexcept
  {
    if (!decodedSize)
    {
      return;
    }

    krys_debug_assert(_decodedSize >= decodedSize);
    _decodedSize -= decodedSize;
    DecodedSizeChanged(-static_cast<int64>(decodedSize));
  }

  void BitmapImageSource::DecodedSizeReset(uint32 decodedSize) noexcept
  {
    krys_debug_assert(_decodedSize >= decodedSize);
    _decodedSize -= decodedSize;

    // Clearing the ImageSource destroys the extra decoded data used for
    // determining image properties.
    decodedSize += _decodedPropertiesSize;
    _decodedPropertiesSize = 0;
    DecodedSizeChanged(-static_cast<int64>(decodedSize));
  }

  void BitmapImageSource::DestroyNativeImageAtIndex(uint32 index) noexcept
  {
    if (index >= _frames.size())
    {
      return;
    }

    DecodedSizeDecreased(_frames[index].ClearImage());
  }

  bool BitmapImageSource::CanDestroyDecodedData() const noexcept
  {
    // Small image should be decoded synchronously. Deleting its decoded frame is fine.
    if (!IsLargeForDecoding())
    {
      return true;
    }

    if (auto imageObserver = GetImageObserver())
    {
      return imageObserver->CanDestroyDecodedData(*_bitmapImage);
    }

    return true;
  }

  void BitmapImageSource::DidDecodeProperties(uint32 decodedPropertiesSize) noexcept
  {
    if (_decodedSize)
    {
      return;
    }

    int64 decodedSize = static_cast<int64>(decodedPropertiesSize) - _decodedPropertiesSize;
    _decodedPropertiesSize = decodedPropertiesSize;
    DecodedSizeChanged(decodedSize);
  }

  void BitmapImageSource::ClearFrameBufferCache() noexcept
  {
    if (!_decoder)
    {
      return;
    }

    _decoder->ClearFrameBufferCache(CurrentFrameIndex());
  }

  EncodedDataStatus BitmapImageSource::SetData(SharedBuffer *data, bool allDataReceived) noexcept
  {
    if (!data)
    {
      return EncodedDataStatus::Unknown;
    }

    RefPtr<ImageDecoder> decoder = ShareRefPtr(Decoder(data));
    if (!decoder)
    {
      return EncodedDataStatus::Unknown;
    }

    decoder->SetData(*data, allDataReceived);
    _allDataReceived = allDataReceived;
    return GetEncodedDataStatus();
  }

  void BitmapImageSource::ResetData() noexcept
  {
    _decoder = nullptr;

    if (_bitmapImage)
    {
      (void)SetData(_bitmapImage->Data(), _allDataReceived);
    }
  }

  ImageFrameAnimator *BitmapImageSource::FrameAnimator() const noexcept
  {
    if (_frameAnimator)
    {
      return _frameAnimator.get();
    }

    // Number of frames can only be known for sure when loading the image is complete.
    if (GetEncodedDataStatus() != EncodedDataStatus::Complete)
    {
      return nullptr;
    }

    if (!IsAnimated())
    {
      return nullptr;
    }

    _frameAnimator = CreateUnique<ImageFrameAnimator>(const_cast<BitmapImageSource &>(*this));
    return _frameAnimator.get();
  }

  void BitmapImageSource::StartAnimation() noexcept
  {
    // A frame's duration is known once it's decoded, so the frame shown first is decoded before its timer
    // starts, as drawing it would.
    if (IsAnimated())
    {
      (void)CurrentNativeImage();
    }
    (void)StartAnimation(SubsamplingLevel::Default, DecodingOptions(DecodingMode::Synchronous));
  }

  bool BitmapImageSource::StartAnimation(SubsamplingLevel subsamplingLevel,
                                         const DecodingOptions &options) noexcept
  {
    auto *frameAnimator = FrameAnimator();
    if (!frameAnimator)
    {
      return false;
    }

    return frameAnimator->StartAnimation(subsamplingLevel, options);
  }

  void BitmapImageSource::StopAnimation() noexcept
  {
    if (!_frameAnimator || !_frameAnimator->IsAnimating())
    {
      return;
    }

    _frameAnimator->StopAnimation();
    StopDecodingWorkQueue();
  }

  void BitmapImageSource::ResetAnimation() noexcept
  {
    if (!_frameAnimator)
    {
      return;
    }

    _frameAnimator->ResetAnimation();
  }

  bool BitmapImageSource::IsAnimating() const noexcept
  {
    return _frameAnimator && _frameAnimator->IsAnimating();
  }

  bool BitmapImageSource::HasEverAnimated() const noexcept
  {
    return _frameAnimator && _frameAnimator->HasEverAnimated();
  }

  bool BitmapImageSource::IsAnimated() const noexcept
  {
    return FrameCount() > 1 && GetRepetitionCount() != RepetitionCountNone;
  }

  bool BitmapImageSource::IsAnimationAllowed() const noexcept
  {
    if (_frameAnimator && !_frameAnimator->IsAnimationAllowed())
    {
      return false;
    }

    // ImageObserver may disallow animation.
    if (auto imageObserver = GetImageObserver())
    {
      return imageObserver->AllowsAnimation(*_bitmapImage);
    }

    return true;
  }

  bool BitmapImageSource::IsLargeForDecoding() const noexcept
  {
    constexpr uint64 KB = 1'024;
    auto sizeInBytes = Size(ImageOrientation::Orientation::None).UnclampedArea() * sizeof(uint32);
    return sizeInBytes > (IsAnimated() ? 100 * KB : 500 * KB);
  }

  void BitmapImageSource::StopDecodingWorkQueue() noexcept
  {
    if (!_workQueue || !_workQueue->IsIdle())
    {
      return;
    }

    _workQueue->Stop();
  }

  bool BitmapImageSource::IsPendingDecodingAtIndex(uint32 index, SubsamplingLevel subsamplingLevel,
                                                   const DecodingOptions &options) const noexcept
  {
    if (!_workQueue)
    {
      return false;
    }

    return _workQueue->IsPendingDecodingAtIndex(index, subsamplingLevel, options);
  }

  bool BitmapImageSource::IsCompatibleWithOptionsAtIndex(uint32 index, SubsamplingLevel subsamplingLevel,
                                                         const DecodingOptions &options) const noexcept
  {
    return FrameAtIndex(index).HasDecodedNativeImageCompatibleWithOptions(options, subsamplingLevel);
  }

  void BitmapImageSource::Decode(Func<void(DecodingStatus)> &&decodeCallback) noexcept
  {
    _decodeCallbacks.push_back(krys::move(decodeCallback));
    uint32 index = CurrentFrameIndex();

    if (IsPendingDecodingAtIndex(index, SubsamplingLevel::Default,
                                 DecodingOptions(DecodingMode::Asynchronous)))
    {
      return;
    }

    bool isCompatibleNativeImage = IsCompatibleWithOptionsAtIndex(
      index, SubsamplingLevel::Default, DecodingOptions(DecodingMode::Asynchronous));

    auto *frameAnimator = FrameAnimator();
    if (frameAnimator && (frameAnimator->HasEverAnimated() || isCompatibleNativeImage))
    {
      // startAnimation() always decodes the nextFrame which is currentFrameIndex + 1.
      // If primaryFrameIndex = 0, then the sequence of decoding is { 1, 2, .., n, 0, 1, ...}.
      if (StartAnimation(SubsamplingLevel::Default, DecodingOptions(DecodingMode::Asynchronous)))
      {
        // Frames decode as they're drawn, so the current one is as decoded as it will be.
        CallDecodeCallbacks(FrameDecodingStatusAtIndex(index));
        return;
      }
    }

    if (!isCompatibleNativeImage && CanDecodeAsynchronously())
    {
      (void)RequestNativeImageAtIndex(index, SubsamplingLevel::Default, ImageAnimatingState::No,
                                      DecodingOptions(DecodingMode::Asynchronous));
      return;
    }

    // Without the work queue, the frame decodes now, and the callbacks are called before this returns.
    auto nativeImage = NativeImageAtIndexCacheIfNeeded(index, SubsamplingLevel::Default,
                                                       DecodingOptions(DecodingMode::Asynchronous));
    CallDecodeCallbacks(nativeImage ? FrameDecodingStatusAtIndex(index) : DecodingStatus::Invalid);
  }

  void BitmapImageSource::CallDecodeCallbacks(DecodingStatus status) noexcept
  {
    if (_decodeCallbacks.empty())
    {
      return;
    }
    auto decodeCallbacks = krys::move(_decodeCallbacks);
    _decodeCallbacks.clear();
    for (auto &decodeCallback : decodeCallbacks)
    {
      decodeCallback(status);
    }
  }

  void BitmapImageSource::ImageFrameAtIndexAvailable(uint32 index, ImageAnimatingState animatingState,
                                                     DecodingStatus decodingStatus) noexcept
  {
    krys_debug_assert(index < FrameCount());
    (void)index;

    // Decode callbacks have to called regardless of the decoding status.
    // Decoding promises have to resolved or rejected.
    CallDecodeCallbacks(decodingStatus);

    if (decodingStatus == DecodingStatus::Invalid)
    {
      return;
    }

    if (auto imageObserver = GetImageObserver())
    {
      imageObserver->ImageFrameAvailable(*_bitmapImage, animatingState, nullptr, decodingStatus);
    }
  }

  void BitmapImageSource::ImageFrameDecodeAtIndexHasFinished(uint32 index, ImageAnimatingState animatingState,
                                                             DecodingStatus decodingStatus) noexcept
  {
    // Depending on its timer, the animator may call or postpone calling imageFrameAvailable().
    if (_frameAnimator
        && _frameAnimator->ImageFrameDecodeAtIndexHasFinished(index, animatingState, decodingStatus))
    {
      return;
    }

    ImageFrameAtIndexAvailable(index, animatingState, decodingStatus);
  }

  void BitmapImageSource::ImageFrameDecodeAtIndexHasFinished(uint32 index, SubsamplingLevel subsamplingLevel,
                                                             ImageAnimatingState animatingState,
                                                             const DecodingOptions &options,
                                                             RefPtr<NativeImage> &&nativeImage) noexcept
  {
    krys_debug_assert(index < _frames.size());

    if (!nativeImage || !_decoder)
    {
      DestroyNativeImageAtIndex(index);
      ImageFrameDecodeAtIndexHasFinished(index, animatingState, DecodingStatus::Invalid);
    }
    else
    {
      Ref<NativeImage> image = krys::move(nativeImage);
      CacheNativeImageAtIndex(index, subsamplingLevel, options, krys::move(image));
      ImageFrameDecodeAtIndexHasFinished(index, animatingState, FrameDecodingStatusAtIndex(index));
    }

    // Do not leave any decoding work queue idle for static images.
    if (animatingState == ImageAnimatingState::No)
    {
      StopDecodingWorkQueue();
    }
  }

  uint32 BitmapImageSource::CurrentFrameIndex() const noexcept
  {
    return _frameAnimator ? _frameAnimator->CurrentFrameIndex() : PrimaryFrameIndex();
  }

  void BitmapImageSource::CacheMetadataAtIndex(uint32 index, SubsamplingLevel subsamplingLevel,
                                               const DecodingOptions &options) noexcept
  {
    krys_debug_assert(_decoder);

    if (index >= _frames.size())
    {
      return;
    }

    auto &frame = _frames[index];

    (void)_decoder->FetchFrameMetaDataAtIndex(index, subsamplingLevel, options, frame);

    if (GetRepetitionCount())
    {
      frame._duration = _decoder->FrameDurationAtIndex(index);
    }
  }

  void BitmapImageSource::CacheNativeImageAtIndex(uint32 index, SubsamplingLevel subsamplingLevel,
                                                  const DecodingOptions &options,
                                                  Ref<NativeImage> &&nativeImage) noexcept
  {
    krys_debug_assert(_decoder);

    if (index >= _frames.size())
    {
      return;
    }

    DestroyNativeImageAtIndex(index);

    // Do not cache NativeImage if adding its sizeInBytes to MemoryCache will cause numerical overflow.
    auto sizeInBytes = nativeImage->Size().UnclampedArea() * sizeof(uint32);
    if (sizeInBytes + _decodedSize > std::numeric_limits<uint32>::max())
    {
      return;
    }

    auto &frame = _frames[index];
    frame._source.Image = krys::move(nativeImage);
    frame._source.Options = options;

    CacheMetadataAtIndex(index, subsamplingLevel, options);
    DecodedSizeIncreased(frame.SizeInBytes());
  }

  const ImageFrame &BitmapImageSource::FrameAtIndex(uint32 index) const noexcept
  {
    if (index >= _frames.size())
    {
      return ImageFrame::DefaultFrame();
    }

    return _frames[index];
  }

  const ImageFrame &
    BitmapImageSource::FrameAtIndexCacheIfNeeded(uint32 index,
                                                 const Maybe<SubsamplingLevel> &subsamplingLevel) noexcept
  {
    if (!_decoder)
    {
      return ImageFrame::DefaultFrame();
    }

    if (index >= _frames.size())
    {
      return ImageFrame::DefaultFrame();
    }

    auto &frame = _frames[index];
    auto subsamplingLevelValue = subsamplingLevel.value_or(frame.GetSubsamplingLevel());

    if (frame.IsComplete() && subsamplingLevelValue == frame.GetSubsamplingLevel())
    {
      return frame;
    }

    DestroyNativeImageAtIndex(index);

    // Retrieve the metadata from ImageDecoder if the ImageFrame isn't complete.
    CacheMetadataAtIndex(index, subsamplingLevelValue, {});
    return frame;
  }

  Expected<Ref<NativeImage>, DecodingStatus>
    BitmapImageSource::NativeImageAtIndexCacheIfNeeded(uint32 index, SubsamplingLevel subsamplingLevel,
                                                       const DecodingOptions &options) noexcept
  {
    if (!_decoder)
    {
      return Unexpected(DecodingStatus::Invalid);
    }

    if (index >= _frames.size())
    {
      return Unexpected(DecodingStatus::Invalid);
    }

    // Never decode the same frame from two different threads.
    if (IsPendingDecodingAtIndex(index, subsamplingLevel, options))
    {
      return Unexpected(DecodingStatus::Decoding);
    }

    if (!IsCompatibleWithOptionsAtIndex(index, subsamplingLevel, options))
    {
      DecodingOptions decodingOptions {DecodingMode::Synchronous};
      PlatformImagePtr platformImage =
        _decoder->CreateFrameImageAtIndex(index, subsamplingLevel, decodingOptions);

      RefPtr<NativeImage> nativeImage = NativeImage::Create(krys::move(platformImage));
      if (!nativeImage)
      {
        return Unexpected(DecodingStatus::Invalid);
      }

      CacheNativeImageAtIndex(index, subsamplingLevel, decodingOptions, krys::move(nativeImage));
    }

    if (RefPtr<NativeImage> nativeImage = FrameAtIndex(index).GetNativeImage())
    {
      return krys::move(nativeImage);
    }

    return Unexpected(DecodingStatus::Invalid);
  }

  DecodingStatus BitmapImageSource::RequestNativeImageAtIndex(uint32 index, SubsamplingLevel subsamplingLevel,
                                                              ImageAnimatingState animatingState,
                                                              const DecodingOptions &options) noexcept
  {
    if (index >= _frames.size())
    {
      return DecodingStatus::Invalid;
    }

    FrameWorkQueue().Dispatch({index, subsamplingLevel, animatingState, options});
    return DecodingStatus::Decoding;
  }

  DecodingStatus BitmapImageSource::RequestNativeImageAtIndexIfNeeded(uint32 index,
                                                                      SubsamplingLevel subsamplingLevel,
                                                                      ImageAnimatingState animatingState,
                                                                      const DecodingOptions &options) noexcept
  {
    if (index >= _frames.size())
    {
      return DecodingStatus::Invalid;
    }

    // Never decode the same frame from two different threads.
    if (IsPendingDecodingAtIndex(index, subsamplingLevel, options))
    {
      return DecodingStatus::Decoding;
    }

    // isCompatibleWithOptionsAtIndex() returns true only if the frame is complete.
    if (IsCompatibleWithOptionsAtIndex(index, subsamplingLevel, options))
    {
      return DecodingStatus::Complete;
    }

    return RequestNativeImageAtIndex(index, subsamplingLevel, animatingState, options);
  }

  Expected<Ref<NativeImage>, DecodingStatus>
    BitmapImageSource::NativeImageAtIndexRequestIfNeeded(uint32 index, SubsamplingLevel subsamplingLevel,
                                                         const DecodingOptions &options) noexcept
  {
    if (!_decoder)
    {
      return Unexpected(DecodingStatus::Invalid);
    }

    auto status =
      RequestNativeImageAtIndexIfNeeded(index, subsamplingLevel, ImageAnimatingState::No, options);
    if (status == DecodingStatus::Invalid || status == DecodingStatus::Decoding)
    {
      return Unexpected(status);
    }

    if (RefPtr<NativeImage> nativeImage = FrameAtIndex(index).GetNativeImage())
    {
      return krys::move(nativeImage);
    }

    return Unexpected(DecodingStatus::Invalid);
  }

  Expected<Ref<NativeImage>, DecodingStatus>
    BitmapImageSource::NativeImageAtIndexForDrawing(uint32 index, SubsamplingLevel subsamplingLevel,
                                                    const DecodingOptions &options) noexcept
  {
    // If this is an animated image and the frame is not available, we have no
    // choice but to decode it synchronously. Otherwise, a flicker will happen.
    // Without the host's work queue, every frame decodes synchronously.
    if (options.GetDecodingMode() == DecodingMode::Asynchronous && !IsAnimated() && CanDecodeAsynchronously())
    {
      return NativeImageAtIndexRequestIfNeeded(index, subsamplingLevel, options);
    }
    return NativeImageAtIndexCacheIfNeeded(index, subsamplingLevel, options);
  }

  Expected<Ref<NativeImage>, DecodingStatus>
    BitmapImageSource::CurrentNativeImageForDrawing(SubsamplingLevel subsamplingLevel,
                                                    const DecodingOptions &options) noexcept
  {
    // The current frame decodes before the animation starts, so its duration (known once it's decoded) times
    // the frame; starting it asks the work queue for the next frame.
    auto nativeImage = NativeImageAtIndexForDrawing(CurrentFrameIndex(), subsamplingLevel, options);
    (void)StartAnimation(subsamplingLevel, options);
    return nativeImage;
  }

  RefPtr<NativeImage> BitmapImageSource::NativeImageAtIndex(uint32 index) noexcept
  {
    auto nativeImage = NativeImageAtIndexCacheIfNeeded(index);
    if (!nativeImage)
    {
      return nullptr;
    }
    return krys::move(*nativeImage);
  }

  RefPtr<NativeImage>
    BitmapImageSource::PreTransformedNativeImageAtIndex(uint32 index, ImageOrientation orientation,
                                                        bool scaleToDensityCorrectedSize) noexcept
  {
    RefPtr<NativeImage> nativeImage = NativeImageAtIndex(index);
    if (!nativeImage)
    {
      return nullptr;
    }

    auto size = Size();
    auto sourceSize = SourceSize();

    if (orientation == ImageOrientation::Orientation::FromImage)
    {
      orientation = FrameOrientationAtIndex(index);
    }

    if (orientation == ImageOrientation::Orientation::None
        && (size == sourceSize || !scaleToDensityCorrectedSize))
    {
      return nativeImage;
    }

    // Laid out as drawn with the orientation, then scaled to the density-corrected size.
    PlatformImagePtr platformImage = nativeImage->GetPlatformImage();
    if (!platformImage || platformImage->Pixels().empty())
    {
      return nativeImage;
    }

    if (orientation != ImageOrientation::Orientation::None)
    {
      auto orientedSize =
        orientation.UsesWidthAsHeight() ? platformImage->Size().TransposedSize() : platformImage->Size();
      platformImage = OrientedPlatformImage(*platformImage, orientation, orientedSize);
    }

    if (auto targetSize = Size(orientation);
        scaleToDensityCorrectedSize && targetSize != platformImage->Size() && !targetSize.IsEmpty())
    {
      platformImage = ScaledPlatformImage(*platformImage, targetSize);
    }
    return NativeImage::Create(krys::move(platformImage));
  }

  IntSize BitmapImageSource::FrameSizeAtIndex(uint32 index, SubsamplingLevel subsamplingLevel) const noexcept
  {
    return const_cast<BitmapImageSource &>(*this).FrameAtIndexCacheIfNeeded(index, subsamplingLevel).Size();
  }

  seconds BitmapImageSource::FrameDurationAtIndex(uint32 index) const noexcept
  {
    return const_cast<BitmapImageSource &>(*this).FrameAtIndexCacheIfNeeded(index).Duration();
  }

  ImageOrientation BitmapImageSource::FrameOrientationAtIndex(uint32 index) const noexcept
  {
    return const_cast<BitmapImageSource &>(*this).FrameAtIndexCacheIfNeeded(index).Orientation();
  }

  DecodingStatus BitmapImageSource::FrameDecodingStatusAtIndex(uint32 index) const noexcept
  {
    return const_cast<BitmapImageSource &>(*this).FrameAtIndexCacheIfNeeded(index).GetDecodingStatus();
  }

  RefPtr<ImageObserver> BitmapImageSource::GetImageObserver() const noexcept
  {
    return _bitmapImage ? _bitmapImage->GetImageObserver() : nullptr;
  }

  utf8_string BitmapImageSource::MimeType() const noexcept
  {
    return _bitmapImage ? _bitmapImage->MimeType() : utf8_string();
  }

  int64 BitmapImageSource::ExpectedContentLength() const noexcept
  {
    return _bitmapImage ? _bitmapImage->ExpectedContentLength() : 0;
  }

  url::URL BitmapImageSource::SourceURL() const noexcept
  {
    return _bitmapImage ? _bitmapImage->SourceURL() : url::URL();
  }

  void BitmapImageSource::Dump(TextStream &ts) const noexcept
  {
    if (_frameAnimator)
    {
      _frameAnimator->Dump(ts);
    }

    _descriptor.Dump(ts);

    ts.DumpProperty(u8"decoded-size", _decodedSize);
  }
}
