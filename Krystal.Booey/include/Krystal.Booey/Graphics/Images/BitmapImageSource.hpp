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

#pragma once

#include "Krystal.Booey/Graphics/Images/BitmapImageDescriptor.hpp"
#include "Krystal.Booey/Graphics/Images/ImageDecoder.hpp"
#include "Krystal.Booey/Graphics/Images/ImageFrameAnimator.hpp"
#include "Krystal.Booey/Graphics/Images/ImageFrameWorkQueue.hpp"
#include "Krystal.Booey/Graphics/Images/ImageSource.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"

namespace krys::boo
{
  class BitmapImage;
  class ImageObserver;

  /// @brief The source of a bitmap image: its decoder, the frames decoded from it and their metadata.
  /// Asynchronous decoding decodes a frame on the host's work queue (`ImageFrameWorkQueue`), once all the
  /// image's data has arrived; without a work queue, or before then, frames decode synchronously. An
  /// animation decodes its next frame on the work queue while the current one shows.
  class BitmapImageSource final : public ImageSource
  {
    friend class ImageFrameAnimator;
    friend class ImageFrameWorkQueue;

  private:
    // State
    WeakPtr<BitmapImage> _bitmapImage;
    AlphaOption _alphaOption {AlphaOption::Premultiplied};
    GammaAndColorProfileOption _gammaAndColorProfileOption {GammaAndColorProfileOption::Applied};
    bool _allDataReceived {false};

    BitmapImageDescriptor _descriptor;
    mutable RefPtr<ImageDecoder> _decoder;
    mutable UniquePtr<ImageFrameAnimator> _frameAnimator;
    mutable RefPtr<ImageFrameWorkQueue> _workQueue;
    List<Func<void(DecodingStatus)>> _decodeCallbacks;

    // ImageFrame
    List<ImageFrame> _frames;
    uint32 _decodedSize {0};
    uint32 _decodedPropertiesSize {0};

  public:
    KRYS_NODISCARD static Ref<BitmapImageSource> Create(BitmapImage &, AlphaOption,
                                                        GammaAndColorProfileOption) noexcept;

    ~BitmapImageSource() noexcept override;

    // State
    KRYS_NODISCARD ImageDecoder *Decoder(SharedBuffer * = nullptr) const noexcept;
    KRYS_NODISCARD ImageFrameAnimator *FrameAnimator() const noexcept;
    KRYS_NODISCARD ImageDecoder *DecoderIfExists() const noexcept
    {
      return _decoder.get();
    }

    // Encoded and decoded data
    void DestroyDecodedData(bool destroyAll) noexcept final;
    void ResetData() noexcept;
    KRYS_NODISCARD uint32 DecodedSize() const noexcept
    {
      return _decodedSize;
    }
    void DidDecodeProperties(uint32 decodedPropertiesSize) noexcept;

    // Animation
    void StartAnimation() noexcept final;
    bool StartAnimation(SubsamplingLevel, const DecodingOptions &) noexcept;
    void StopAnimation() noexcept final;
    void ResetAnimation() noexcept final;
    KRYS_NODISCARD bool IsAnimating() const noexcept final;
    KRYS_NODISCARD bool IsAnimationAllowed() const noexcept;
    KRYS_NODISCARD bool HasEverAnimated() const noexcept final;

    // Decoding
    KRYS_NODISCARD bool IsPendingDecodingAtIndex(uint32 index, SubsamplingLevel,
                                                 const DecodingOptions &) const noexcept;
    void ImageFrameAtIndexAvailable(uint32 index, ImageAnimatingState, DecodingStatus) noexcept;
    void ImageFrameDecodeAtIndexHasFinished(uint32 index, ImageAnimatingState, DecodingStatus) noexcept;
    /// @brief The frame at @p index, asked for with @p options at @p subsamplingLevel, was decoded on the
    /// work queue as @p nativeImage, or failed to (null).
    void ImageFrameDecodeAtIndexHasFinished(uint32 index, SubsamplingLevel subsamplingLevel,
                                            ImageAnimatingState animatingState,
                                            const DecodingOptions &options,
                                            RefPtr<NativeImage> &&nativeImage) noexcept;
    KRYS_NODISCARD uint32 CurrentFrameIndex() const noexcept final;

    // Decoding & animation
    void DestroyNativeImageAtIndex(uint32 index) noexcept;

    // ImageFrame
    KRYS_NODISCARD uint32 PrimaryFrameIndex() const noexcept final
    {
      return _descriptor.PrimaryFrameIndex();
    }

    KRYS_NODISCARD const List<ImageFrame> &Frames() const noexcept
    {
      return _frames;
    }
    const ImageFrame &PrimaryImageFrame(const Maybe<SubsamplingLevel> &subsamplingLevel = null) noexcept final
    {
      return FrameAtIndexCacheIfNeeded(PrimaryFrameIndex(), subsamplingLevel);
    }

    // NativeImage
    KRYS_NODISCARD RefPtr<NativeImage> PrimaryNativeImageIfExists() const noexcept
    {
      return FrameAtIndex(PrimaryFrameIndex()).GetNativeImage();
    }
    KRYS_NODISCARD RefPtr<NativeImage> PrimaryNativeImage() noexcept final
    {
      return NativeImageAtIndex(PrimaryFrameIndex());
    }

    // Image Metadata
    KRYS_NODISCARD uint32 FrameCount() const noexcept final
    {
      return _descriptor.FrameCount();
    }
    KRYS_NODISCARD RepetitionCount GetRepetitionCount() const noexcept
    {
      return _descriptor.GetRepetitionCount();
    }

    // ImageFrame metadata
    KRYS_NODISCARD IntSize FrameSizeAtIndex(uint32 index,
                                            SubsamplingLevel = SubsamplingLevel::Default) const noexcept;
    KRYS_NODISCARD seconds FrameDurationAtIndex(uint32 index) const noexcept final;
    KRYS_NODISCARD DecodingStatus FrameDecodingStatusAtIndex(uint32 index) const noexcept final;

    KRYS_NODISCARD url::URL SourceURL() const noexcept;

  private:
    BitmapImageSource(BitmapImage &, AlphaOption, GammaAndColorProfileOption) noexcept;

    // Encoded and decoded data
    void EncodedDataStatusChanged(EncodedDataStatus) noexcept;
    EncodedDataStatus DataChanged(SharedBuffer *, bool allDataReceived) noexcept final;
    EncodedDataStatus SetData(SharedBuffer *, bool allDataReceived) noexcept;

    void DecodedSizeChanged(int64 decodedSize) noexcept;
    void DecodedSizeIncreased(uint32 decodedSize) noexcept;
    void DecodedSizeDecreased(uint32 decodedSize) noexcept;
    void DecodedSizeReset(uint32 decodedSize) noexcept;
    KRYS_NODISCARD bool CanDestroyDecodedData() const noexcept;

    void ClearFrameBufferCache() noexcept;

    // Animation
    KRYS_NODISCARD bool IsAnimated() const noexcept final;

    // Decoding
    KRYS_NODISCARD bool IsLargeForDecoding() const noexcept final;
    KRYS_NODISCARD bool IsCompatibleWithOptionsAtIndex(uint32 index, SubsamplingLevel,
                                                       const DecodingOptions &) const noexcept;
    KRYS_NODISCARD ImageFrameWorkQueue &FrameWorkQueue() const noexcept;
    /// @brief Whether frames can decode on the host's work queue: it has one, and all the data has arrived.
    KRYS_NODISCARD bool CanDecodeAsynchronously() const noexcept;
    /// @brief A decoder of the work queue's own, over all the image's data; null if there's none to decode.
    KRYS_NODISCARD RefPtr<ImageDecoder> CreateWorkQueueDecoder() const noexcept;
    KRYS_NODISCARD BitmapImage *Image() const noexcept
    {
      return _bitmapImage.get();
    }
    DecodingStatus RequestNativeImageAtIndex(uint32 index, SubsamplingLevel, ImageAnimatingState,
                                             const DecodingOptions &) noexcept;
    DecodingStatus RequestNativeImageAtIndexIfNeeded(uint32 index, SubsamplingLevel, ImageAnimatingState,
                                                     const DecodingOptions &) noexcept;
    void StopDecodingWorkQueue() noexcept final;
    void Decode(Func<void(DecodingStatus)> &&decodeCallback) noexcept final;
    void CallDecodeCallbacks(DecodingStatus) noexcept;

    // ImageFrame

    void CacheMetadataAtIndex(uint32 index, SubsamplingLevel, const DecodingOptions &) noexcept;
    void CacheNativeImageAtIndex(uint32 index, SubsamplingLevel, const DecodingOptions &,
                                 Ref<NativeImage> &&) noexcept;

    KRYS_NODISCARD const ImageFrame &FrameAtIndex(uint32 index) const noexcept;
    const ImageFrame &FrameAtIndexCacheIfNeeded(uint32 index,
                                                const Maybe<SubsamplingLevel> & = null) noexcept;
    const ImageFrame &CurrentImageFrame(const Maybe<SubsamplingLevel> &subsamplingLevel = null) noexcept final
    {
      return FrameAtIndexCacheIfNeeded(CurrentFrameIndex(), subsamplingLevel);
    }

    // NativeImage
    KRYS_NODISCARD Expected<Ref<NativeImage>, DecodingStatus>
      NativeImageAtIndexCacheIfNeeded(uint32 index, SubsamplingLevel = SubsamplingLevel::Default,
                                      const DecodingOptions & = {}) noexcept;
    KRYS_NODISCARD Expected<Ref<NativeImage>, DecodingStatus>
      NativeImageAtIndexRequestIfNeeded(uint32 index, SubsamplingLevel, const DecodingOptions &) noexcept;
    KRYS_NODISCARD Expected<Ref<NativeImage>, DecodingStatus>
      NativeImageAtIndexForDrawing(uint32 index, SubsamplingLevel, const DecodingOptions &) noexcept;

    KRYS_NODISCARD Expected<Ref<NativeImage>, DecodingStatus>
      CurrentNativeImageForDrawing(SubsamplingLevel, const DecodingOptions &) noexcept final;

    KRYS_NODISCARD RefPtr<NativeImage> NativeImageAtIndex(uint32 index) noexcept final;
    /// @brief The frame at @p index laid out as drawn with the orientation and, if
    /// @p scaleToDensityCorrectedSize, resampled to the density-corrected size.
    KRYS_NODISCARD RefPtr<NativeImage>
      PreTransformedNativeImageAtIndex(uint32 index, ImageOrientation,
                                       bool scaleToDensityCorrectedSize = true) noexcept;

    KRYS_NODISCARD RefPtr<NativeImage> CurrentNativeImage() noexcept final
    {
      return NativeImageAtIndex(CurrentFrameIndex());
    }
    KRYS_NODISCARD RefPtr<NativeImage>
      CurrentPreTransformedNativeImage(ImageOrientation orientation) noexcept final
    {
      return PreTransformedNativeImageAtIndex(CurrentFrameIndex(), orientation);
    }
    KRYS_NODISCARD RefPtr<NativeImage> CurrentOrientedNativeImage(ImageOrientation orientation) noexcept final
    {
      return PreTransformedNativeImageAtIndex(CurrentFrameIndex(), orientation, false);
    }

    KRYS_NODISCARD EncodedDataStatus GetEncodedDataStatus() const noexcept
    {
      return _descriptor.GetEncodedDataStatus();
    }
    KRYS_NODISCARD IntSize
      Size(ImageOrientation orientation = ImageOrientation::Orientation::FromImage) const noexcept final
    {
      return _descriptor.Size(orientation);
    }
    KRYS_NODISCARD IntSize
      SourceSize(ImageOrientation orientation = ImageOrientation::Orientation::FromImage) const noexcept final
    {
      return _descriptor.SourceSize(orientation);
    }
    KRYS_NODISCARD Maybe<IntSize> DensityCorrectedSize() const noexcept
    {
      return _descriptor.DensityCorrectedSize();
    }
    KRYS_NODISCARD bool HasDensityCorrectedSize() const noexcept final
    {
      return DensityCorrectedSize().has_value();
    }
    KRYS_NODISCARD ImageOrientation Orientation() const noexcept final
    {
      return _descriptor.Orientation();
    }
    KRYS_NODISCARD Maybe<Color> SinglePixelSolidColor() const noexcept final
    {
      return _descriptor.SinglePixelSolidColor();
    }

    KRYS_NODISCARD utf8_string FilenameExtension() const noexcept final
    {
      return _descriptor.FilenameExtension();
    }
    KRYS_NODISCARD utf8_string AccessibilityDescription() const noexcept final
    {
      return _descriptor.AccessibilityDescription();
    }
    KRYS_NODISCARD Maybe<IntPoint> HotSpot() const noexcept final
    {
      return _descriptor.HotSpot();
    }
    KRYS_NODISCARD SubsamplingLevel MaximumSubsamplingLevel() const noexcept
    {
      return _descriptor.MaximumSubsamplingLevel();
    }
    KRYS_NODISCARD SubsamplingLevel
      SubsamplingLevelForScaleFactor(GraphicsContext &context, const FloatSize &scaleFactor,
                                     AllowImageSubsampling allowImageSubsampling) noexcept final
    {
      return _descriptor.SubsamplingLevelForScaleFactor(context, scaleFactor, allowImageSubsampling);
    }

    // ImageFrame metadata
    KRYS_NODISCARD ImageOrientation FrameOrientationAtIndex(uint32 index) const noexcept final;

    // BitmapImage metadata
    KRYS_NODISCARD RefPtr<ImageObserver> GetImageObserver() const noexcept;
    KRYS_NODISCARD utf8_string MimeType() const noexcept;
    KRYS_NODISCARD int64 ExpectedContentLength() const noexcept;

    void Dump(TextStream &) const noexcept final;
  };
}
