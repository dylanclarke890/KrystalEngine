#include "Krystal.Booey/Graphics/BitmapImageDescriptor.hpp"
#include "Krystal.Booey/Graphics/BitmapImageSource.hpp"
#include "Krystal.Booey/Graphics/GraphicsContext.hpp"
#include "Krystal.Booey/Graphics/ImageDecoder.hpp"
#include "Krystal.Core/Text/TextStream.hpp"

namespace krys::boo
{
  BitmapImageDescriptor::BitmapImageDescriptor(BitmapImageSource &source) noexcept : _source(source)
  {
  }

  template <typename TMetadata>
  TMetadata BitmapImageDescriptor::ImageMetadata(TMetadata &cachedValue, const TMetadata &defaultValue,
                                                 CachedFlag cachedFlag,
                                                 TMetadata (ImageDecoder::*functor)()
                                                   const noexcept) const noexcept
  {
    if (IsCached(cachedFlag))
    {
      return cachedValue;
    }

    RefPtr<ImageDecoder> decoder = ShareRefPtr(_source.DecoderIfExists());
    if (!decoder)
    {
      return defaultValue;
    }

    if (!IsSizeAvailable())
    {
      return defaultValue;
    }

    cachedValue = ((*decoder).*functor)();
    SetCached(cachedFlag);
    _source.DidDecodeProperties(static_cast<uint32>(decoder->BytesDecodedToDetermineProperties()));
    return cachedValue;
  }

  template <typename TMetadata>
  TMetadata BitmapImageDescriptor::PrimaryImageFrameMetadata(
    TMetadata &cachedValue, CachedFlag cachedFlag, TMetadata (ImageFrame::*functor)() const noexcept,
    const Maybe<SubsamplingLevel> &subsamplingLevel) const noexcept
  {
    if (IsCached(cachedFlag))
    {
      return cachedValue;
    }

    auto &frame = _source.PrimaryImageFrame(subsamplingLevel);

    // Don't cache any unavailable frame metadata. Just return the default metadata.
    if (!frame.HasMetadata())
    {
      return (frame.*functor)();
    }

    cachedValue = (frame.*functor)();
    SetCached(cachedFlag);
    return cachedValue;
  }

  template <typename TMetadata>
  TMetadata BitmapImageDescriptor::PrimaryNativeImageMetadata(TMetadata &cachedValue,
                                                              const TMetadata &defaultValue,
                                                              CachedFlag cachedFlag,
                                                              TMetadata (NativeImage::*functor)()
                                                                const noexcept) const noexcept
  {
    if (IsCached(cachedFlag))
    {
      return cachedValue;
    }

    RefPtr<NativeImage> nativeImage = _source.PrimaryNativeImage();
    if (!nativeImage)
    {
      return defaultValue;
    }

    cachedValue = ((*nativeImage).*functor)();
    SetCached(cachedFlag);
    return cachedValue;
  }

  EncodedDataStatus BitmapImageDescriptor::GetEncodedDataStatus() const noexcept
  {
    if (IsCached(CachedFlag::EncodedDataStatus))
    {
      return _encodedDataStatus;
    }

    RefPtr<ImageDecoder> decoder = ShareRefPtr(_source.DecoderIfExists());
    if (!decoder)
    {
      return EncodedDataStatus::Unknown;
    }

    _encodedDataStatus = decoder->GetEncodedDataStatus();
    SetCached(CachedFlag::EncodedDataStatus);

    if (_encodedDataStatus >= EncodedDataStatus::SizeAvailable)
    {
      _source.DidDecodeProperties(static_cast<uint32>(decoder->BytesDecodedToDetermineProperties()));
    }

    return _encodedDataStatus;
  }

  IntSize BitmapImageDescriptor::Size(ImageOrientation orientation) const noexcept
  {
    auto densityCorrectedSize = DensityCorrectedSize();
    if (!densityCorrectedSize)
    {
      return SourceSize(orientation);
    }

    if (orientation == ImageOrientation::Orientation::FromImage)
    {
      orientation = Orientation();
    }

    return orientation.UsesWidthAsHeight() ? densityCorrectedSize->TransposedSize() : *densityCorrectedSize;
  }

  IntSize BitmapImageDescriptor::SourceSize(ImageOrientation orientation) const noexcept
  {
    IntSize size;

    // It's possible that we have decoded the metadata, but not frame contents yet. In that case ImageDecoder
    // claims to have the size available, but the frame cache is empty. Return the decoder size without
    // caching in such case.
    RefPtr<ImageDecoder> decoder = ShareRefPtr(_source.DecoderIfExists());
    if (decoder && _source.Frames().empty())
    {
      size = decoder->Size();
    }
    else
    {
      size = PrimaryImageFrameMetadata(_size, CachedFlag::Size, &ImageFrame::Size, SubsamplingLevel::Default);
    }

    if (orientation == ImageOrientation::Orientation::FromImage)
    {
      orientation = Orientation();
    }

    return orientation.UsesWidthAsHeight() ? size.TransposedSize() : size;
  }

  Maybe<IntSize> BitmapImageDescriptor::DensityCorrectedSize() const noexcept
  {
    return PrimaryImageFrameMetadata(_densityCorrectedSize, CachedFlag::DensityCorrectedSize,
                                     &ImageFrame::DensityCorrectedSize, SubsamplingLevel::Default);
  }

  ImageOrientation BitmapImageDescriptor::Orientation() const noexcept
  {
    return PrimaryImageFrameMetadata(_orientation, CachedFlag::Orientation, &ImageFrame::Orientation);
  }

  uint32 BitmapImageDescriptor::PrimaryFrameIndex() const noexcept
  {
    return static_cast<uint32>(ImageMetadata(_primaryFrameIndex, size_t(0), CachedFlag::PrimaryFrameIndex,
                                             &ImageDecoder::PrimaryFrameIndex));
  }

  uint32 BitmapImageDescriptor::FrameCount() const noexcept
  {
    return static_cast<uint32>(
      ImageMetadata(_frameCount, size_t(0), CachedFlag::FrameCount, &ImageDecoder::FrameCount));
  }

  RepetitionCount BitmapImageDescriptor::GetRepetitionCount() const noexcept
  {
    return ImageMetadata(_repetitionCount, static_cast<RepetitionCount>(RepetitionCountNone),
                         CachedFlag::RepetitionCount, &ImageDecoder::GetRepetitionCount);
  }

  Maybe<Color> BitmapImageDescriptor::SinglePixelSolidColor() const noexcept
  {
    if (!_source.HasSolidColor())
    {
      return null;
    }

    return PrimaryNativeImageMetadata(_singlePixelSolidColor, Maybe<Color>(),
                                      CachedFlag::SinglePixelSolidColor, &NativeImage::SinglePixelSolidColor);
  }

  utf8_string BitmapImageDescriptor::FilenameExtension() const noexcept
  {
    return ImageMetadata(_filenameExtension, utf8_string(), CachedFlag::FilenameExtension,
                         &ImageDecoder::FilenameExtension);
  }

  utf8_string BitmapImageDescriptor::AccessibilityDescription() const noexcept
  {
    return ImageMetadata(_accessibilityDescription, utf8_string(), CachedFlag::AccessibilityDescription,
                         &ImageDecoder::AccessibilityDescription);
  }

  Maybe<IntPoint> BitmapImageDescriptor::HotSpot() const noexcept
  {
    return ImageMetadata(_hotSpot, Maybe<IntPoint>(), CachedFlag::HotSpot, &ImageDecoder::HotSpot);
  }

  SubsamplingLevel BitmapImageDescriptor::MaximumSubsamplingLevel() const noexcept
  {
    if (IsCached(CachedFlag::MaximumSubsamplingLevel))
    {
      return _maximumSubsamplingLevel;
    }

    RefPtr<ImageDecoder> decoder = ShareRefPtr(_source.DecoderIfExists());
    if (!decoder)
    {
      return SubsamplingLevel::Default;
    }

    if (!IsSizeAvailable())
    {
      return SubsamplingLevel::Default;
    }

    // FIXME: this value was chosen to be appropriate for Apple ports since the image
    // subsampling is only enabled by default on Apple ports. Choose a different value
    // if image subsampling is enabled on other platform.
    static constexpr int64 maximumImageAreaBeforeSubsampling = 5 * 1'024 * 1'024;
    auto level = SubsamplingLevel::First;

    for (; level < SubsamplingLevel::Last;
         level = static_cast<SubsamplingLevel>(static_cast<uint8>(level) + 1))
    {
      auto area = _source.FrameSizeAtIndex(0, level).UnclampedArea();
      if (area < maximumImageAreaBeforeSubsampling)
      {
        break;
      }
    }

    _maximumSubsamplingLevel = level;
    SetCached(CachedFlag::MaximumSubsamplingLevel);
    return _maximumSubsamplingLevel;
  }

  SubsamplingLevel BitmapImageDescriptor::SubsamplingLevelForScaleFactor(GraphicsContext &, const FloatSize &,
                                                                         AllowImageSubsampling) const noexcept
  {
    // Images are subsampled only where the platform decoder can decode at a lower resolution.
    return SubsamplingLevel::Default;
  }

  void BitmapImageDescriptor::Dump(TextStream &ts) const noexcept
  {
    ts.DumpProperty(u8"size", Size());
    ts.DumpProperty(u8"density-corrected-size", DensityCorrectedSize());
    ts.DumpProperty(u8"primary-frame-index", PrimaryFrameIndex());
    ts.DumpProperty(u8"frame-count", FrameCount());
    ts.DumpProperty(u8"repetition-count", GetRepetitionCount());
    ts.DumpProperty(u8"filename-extension", FilenameExtension());
    ts.DumpProperty(u8"accessibility-description", AccessibilityDescription());
  }
}
