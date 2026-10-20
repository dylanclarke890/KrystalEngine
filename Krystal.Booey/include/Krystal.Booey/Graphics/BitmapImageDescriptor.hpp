#pragma once

#include "Krystal.Booey/Graphics/Color/Color.hpp"
#include "Krystal.Booey/Graphics/ImageOrientation.hpp"
#include "Krystal.Booey/Graphics/ImageTypes.hpp"
#include "Krystal.Booey/Graphics/IntPoint.hpp"
#include "Krystal.Booey/Graphics/IntSize.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/String.hpp"

namespace krys
{
  class TextStream;
}

namespace krys::boo
{
  class BitmapImageSource;
  class FloatSize;
  class GraphicsContext;
  class ImageDecoder;
  class ImageFrame;
  class NativeImage;

  /// @brief A bitmap image's metadata, read from its decoder, primary frame or primary native image once
  /// they have it, and cached until the data changes.
  // FIXME: The colour space, HDR gain map and UTI, once there are colour spaces and HDR.
  class BitmapImageDescriptor
  {
  private:
    enum class CachedFlag : uint16
    {
      EncodedDataStatus = 1 << 0,
      Size = 1 << 1,
      DensityCorrectedSize = 1 << 2,
      Orientation = 1 << 3,
      PrimaryFrameIndex = 1 << 4,
      FrameCount = 1 << 5,
      RepetitionCount = 1 << 6,
      SinglePixelSolidColor = 1 << 7,
      FilenameExtension = 1 << 8,
      AccessibilityDescription = 1 << 9,
      HotSpot = 1 << 10,
      MaximumSubsamplingLevel = 1 << 11,
    };

    mutable uint16 _cachedFlags {0};

    mutable EncodedDataStatus _encodedDataStatus {EncodedDataStatus::Unknown};
    mutable IntSize _size;
    mutable Maybe<IntSize> _densityCorrectedSize;
    mutable ImageOrientation _orientation;
    mutable size_t _primaryFrameIndex {0};
    mutable size_t _frameCount {0};
    mutable RepetitionCount _repetitionCount {RepetitionCountNone};
    mutable Maybe<Color> _singlePixelSolidColor;
    mutable utf8_string _filenameExtension;
    mutable utf8_string _accessibilityDescription;
    mutable Maybe<IntPoint> _hotSpot;
    mutable SubsamplingLevel _maximumSubsamplingLevel {SubsamplingLevel::Default};

    BitmapImageSource &_source;

  public:
    explicit BitmapImageDescriptor(BitmapImageSource &) noexcept;

    void Clear() noexcept
    {
      _cachedFlags = 0;
    }

    KRYS_NODISCARD EncodedDataStatus GetEncodedDataStatus() const noexcept;
    KRYS_NODISCARD bool IsSizeAvailable() const noexcept
    {
      return GetEncodedDataStatus() >= EncodedDataStatus::SizeAvailable;
    }
    KRYS_NODISCARD IntSize Size(ImageOrientation = ImageOrientation::Orientation::FromImage) const noexcept;
    KRYS_NODISCARD
    IntSize SourceSize(ImageOrientation = ImageOrientation::Orientation::FromImage) const noexcept;
    KRYS_NODISCARD Maybe<IntSize> DensityCorrectedSize() const noexcept;
    KRYS_NODISCARD ImageOrientation Orientation() const noexcept;
    KRYS_NODISCARD uint32 PrimaryFrameIndex() const noexcept;
    KRYS_NODISCARD uint32 FrameCount() const noexcept;
    KRYS_NODISCARD RepetitionCount GetRepetitionCount() const noexcept;
    KRYS_NODISCARD Maybe<Color> SinglePixelSolidColor() const noexcept;

    KRYS_NODISCARD utf8_string FilenameExtension() const noexcept;
    KRYS_NODISCARD utf8_string AccessibilityDescription() const noexcept;
    KRYS_NODISCARD Maybe<IntPoint> HotSpot() const noexcept;
    KRYS_NODISCARD SubsamplingLevel MaximumSubsamplingLevel() const noexcept;

    KRYS_NODISCARD SubsamplingLevel SubsamplingLevelForScaleFactor(GraphicsContext &,
                                                                   const FloatSize &scaleFactor,
                                                                   AllowImageSubsampling) const noexcept;

    void Dump(TextStream &) const noexcept;

  private:
    KRYS_NODISCARD bool IsCached(CachedFlag flag) const noexcept
    {
      return _cachedFlags & static_cast<uint16>(flag);
    }
    void SetCached(CachedFlag flag) const noexcept
    {
      _cachedFlags |= static_cast<uint16>(flag);
    }

    template <typename TMetadata>
    TMetadata ImageMetadata(TMetadata &cachedValue, const TMetadata &defaultValue, CachedFlag,
                            TMetadata (ImageDecoder::*functor)() const noexcept) const noexcept;

    template <typename TMetadata>
    TMetadata PrimaryImageFrameMetadata(TMetadata &cachedValue, CachedFlag,
                                        TMetadata (ImageFrame::*functor)() const noexcept,
                                        const Maybe<SubsamplingLevel> & = null) const noexcept;

    template <typename TMetadata>
    TMetadata PrimaryNativeImageMetadata(TMetadata &cachedValue, const TMetadata &defaultValue, CachedFlag,
                                         TMetadata (NativeImage::*functor)() const noexcept) const noexcept;
  };
}
