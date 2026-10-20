#pragma once

#include "Krystal.Booey/Graphics/Color/Color.hpp"
#include "Krystal.Booey/Graphics/DecodingOptions.hpp"
#include "Krystal.Booey/Graphics/ImageFrame.hpp"
#include "Krystal.Booey/Graphics/ImageOrientation.hpp"
#include "Krystal.Booey/Graphics/ImageTypes.hpp"
#include "Krystal.Booey/Graphics/IntPoint.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/CanMakeWeakPtr.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Types/Expected.hpp"
#include "Krystal.Core/Types/Func.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/String.hpp"

namespace krys
{
  class TextStream;
}

namespace krys::boo
{
  class FloatSize;
  class GraphicsContext;
  class SharedBuffer;

  /// @brief Where an image's frames and metadata come from: its decoder, or a native image it was made from.
  // FIXME: Colour spaces and HDR, once there are any.
  class ImageSource : public RefCounted<ImageSource>, public CanMakeWeakPtr<ImageSource>
  {
  public:
    virtual ~ImageSource() noexcept = default;

    // Encoded and decoded data
    virtual EncodedDataStatus DataChanged(SharedBuffer *, bool) noexcept
    {
      krys_assert_msg(false, "Not reached");
      return EncodedDataStatus::Unknown;
    }
    virtual void DestroyDecodedData(bool) noexcept
    {
      krys_assert_msg(false, "Not reached");
    }

    // Animation
    virtual void StartAnimation() noexcept
    {
    }
    virtual void StopAnimation() noexcept
    {
    }
    virtual void ResetAnimation() noexcept
    {
    }
    KRYS_NODISCARD virtual bool IsAnimated() const noexcept
    {
      return false;
    }
    KRYS_NODISCARD virtual bool IsAnimating() const noexcept
    {
      return false;
    }
    KRYS_NODISCARD virtual bool HasEverAnimated() const noexcept
    {
      return false;
    }

    // Decoding
    KRYS_NODISCARD virtual bool IsLargeForDecoding() const noexcept
    {
      return false;
    }
    virtual void StopDecodingWorkQueue() noexcept
    {
      krys_assert_msg(false, "Not reached");
    }
    virtual void Decode(Func<void(DecodingStatus)> &&) noexcept
    {
      krys_assert_msg(false, "Not reached");
    }

    // ImageFrame
    KRYS_NODISCARD virtual uint32 CurrentFrameIndex() const noexcept
    {
      return PrimaryFrameIndex();
    }

    virtual const ImageFrame &PrimaryImageFrame(const Maybe<SubsamplingLevel> & = null) noexcept = 0;
    virtual const ImageFrame &
      CurrentImageFrame(const Maybe<SubsamplingLevel> &subsamplingLevel = null) noexcept
    {
      return PrimaryImageFrame(subsamplingLevel);
    }

    // NativeImage
    KRYS_NODISCARD virtual RefPtr<NativeImage> PrimaryNativeImage() noexcept = 0;
    KRYS_NODISCARD virtual RefPtr<NativeImage> CurrentNativeImage() noexcept
    {
      return PrimaryNativeImage();
    }
    KRYS_NODISCARD virtual RefPtr<NativeImage> CurrentPreTransformedNativeImage(ImageOrientation) noexcept
    {
      return CurrentNativeImage();
    }

    KRYS_NODISCARD virtual RefPtr<NativeImage> NativeImageAtIndex(uint32) noexcept
    {
      return PrimaryNativeImage();
    }

    KRYS_NODISCARD virtual Expected<Ref<NativeImage>, DecodingStatus>
      PrimaryNativeImageForDrawing(SubsamplingLevel, const DecodingOptions &) noexcept;
    KRYS_NODISCARD virtual Expected<Ref<NativeImage>, DecodingStatus>
      CurrentNativeImageForDrawing(SubsamplingLevel, const DecodingOptions &) noexcept;

    // Image Metadata
    KRYS_NODISCARD virtual IntSize
      Size(ImageOrientation = ImageOrientation::Orientation::FromImage) const noexcept = 0;
    KRYS_NODISCARD virtual IntSize
      SourceSize(ImageOrientation orientation = ImageOrientation::Orientation::FromImage) const noexcept
    {
      return Size(orientation);
    }
    KRYS_NODISCARD virtual bool HasDensityCorrectedSize() const noexcept
    {
      return false;
    }
    KRYS_NODISCARD virtual ImageOrientation Orientation() const noexcept
    {
      return ImageOrientation::Orientation::None;
    }
    KRYS_NODISCARD virtual uint32 PrimaryFrameIndex() const noexcept
    {
      return 0;
    }
    KRYS_NODISCARD virtual uint32 FrameCount() const noexcept
    {
      return 1;
    }
    KRYS_NODISCARD virtual Maybe<Color> SinglePixelSolidColor() const noexcept = 0;

    KRYS_NODISCARD bool HasSolidColor() const noexcept;

    KRYS_NODISCARD virtual utf8_string FilenameExtension() const noexcept
    {
      return {};
    }
    KRYS_NODISCARD virtual utf8_string AccessibilityDescription() const noexcept
    {
      return {};
    }
    KRYS_NODISCARD virtual Maybe<IntPoint> HotSpot() const noexcept
    {
      return null;
    }

    KRYS_NODISCARD virtual SubsamplingLevel
      SubsamplingLevelForScaleFactor(GraphicsContext &, const FloatSize &, AllowImageSubsampling) noexcept
    {
      return SubsamplingLevel::Default;
    }

    // ImageFrame Metadata
    KRYS_NODISCARD virtual seconds FrameDurationAtIndex(uint32) const noexcept
    {
      krys_assert_msg(false, "Not reached");
      return {};
    }
    KRYS_NODISCARD virtual ImageOrientation FrameOrientationAtIndex(uint32) const noexcept
    {
      krys_assert_msg(false, "Not reached");
      return ImageOrientation::Orientation::None;
    }
    KRYS_NODISCARD virtual DecodingStatus FrameDecodingStatusAtIndex(uint32) const noexcept
    {
      krys_assert_msg(false, "Not reached");
      return DecodingStatus::Invalid;
    }

    virtual void Dump(TextStream &) const noexcept
    {
    }
  };
}
