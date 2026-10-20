#pragma once

#include "Krystal.Booey/Graphics/BitmapImageDescriptor.hpp"
#include "Krystal.Booey/Graphics/ImageDecoder.hpp"
#include "Krystal.Booey/Graphics/ImageSource.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"

namespace krys::boo
{
  class BitmapImage;
  class ImageObserver;

  /// @brief The source of a bitmap image: its decoder, the frames decoded from it and their metadata.
  /// FIXME: The decoding work queue: asynchronous decoding decodes synchronously for now. The frame
  /// animator, and with it animated images, come in G7d.
  class BitmapImageSource final : public ImageSource
  {
  private:
    // State
    WeakPtr<BitmapImage> _bitmapImage;
    AlphaOption _alphaOption {AlphaOption::Premultiplied};
    GammaAndColorProfileOption _gammaAndColorProfileOption {GammaAndColorProfileOption::Applied};
    bool _allDataReceived {false};

    BitmapImageDescriptor _descriptor;
    mutable RefPtr<ImageDecoder> _decoder;
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
    KRYS_NODISCARD bool IsAnimationAllowed() const noexcept;

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
    void StopDecodingWorkQueue() noexcept final;
    void Decode(Func<void(DecodingStatus)> &&decodeCallback) noexcept final;
    void CallDecodeCallbacks(DecodingStatus) noexcept;

    // ImageFrame
    KRYS_NODISCARD uint32 CurrentFrameIndex() const noexcept final;

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
      NativeImageAtIndexForDrawing(uint32 index, SubsamplingLevel, const DecodingOptions &) noexcept;

    KRYS_NODISCARD Expected<Ref<NativeImage>, DecodingStatus>
      CurrentNativeImageForDrawing(SubsamplingLevel, const DecodingOptions &) noexcept final;

    KRYS_NODISCARD RefPtr<NativeImage> NativeImageAtIndex(uint32 index) noexcept final;
    KRYS_NODISCARD RefPtr<NativeImage> PreTransformedNativeImageAtIndex(uint32 index,
                                                                        ImageOrientation) noexcept;

    KRYS_NODISCARD RefPtr<NativeImage> CurrentNativeImage() noexcept final
    {
      return NativeImageAtIndex(CurrentFrameIndex());
    }
    KRYS_NODISCARD RefPtr<NativeImage>
      CurrentPreTransformedNativeImage(ImageOrientation orientation) noexcept final
    {
      return PreTransformedNativeImageAtIndex(CurrentFrameIndex(), orientation);
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
