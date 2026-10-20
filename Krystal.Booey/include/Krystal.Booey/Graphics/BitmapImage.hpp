#pragma once

#include "Krystal.Booey/Graphics/Image.hpp"
#include "Krystal.Booey/Graphics/ImageSource.hpp"
#include "Krystal.Core/TypeCast.hpp"

namespace krys::boo
{
  class NativeImage;

  /// @brief An image decoded from encoded data (PNG, JPEG, WebP, GIF, BMP or ICO), or made from a native
  /// image.
  class BitmapImage final : public Image
  {
  private:
    Ref<ImageSource> _source;

  public:
    KRYS_NODISCARD static Ref<BitmapImage>
      Create(ImageObserver * = nullptr, AlphaOption = AlphaOption::Premultiplied,
             GammaAndColorProfileOption = GammaAndColorProfileOption::Applied) noexcept;
    KRYS_NODISCARD static Ref<BitmapImage> Create(Ref<NativeImage> &&) noexcept;
    KRYS_NODISCARD static RefPtr<BitmapImage> Create(RefPtr<NativeImage> &&) noexcept;
    KRYS_NODISCARD static RefPtr<BitmapImage> Create(PlatformImagePtr &&) noexcept;

    // Animation
    void StartAnimation() noexcept final
    {
      _source->StartAnimation();
    }
    void StopAnimation() noexcept final
    {
      _source->StopAnimation();
    }
    void ResetAnimation() noexcept final
    {
      _source->ResetAnimation();
    }
    KRYS_NODISCARD bool IsAnimated() const noexcept final
    {
      return _source->IsAnimated();
    }
    KRYS_NODISCARD bool HasEverAnimated() const noexcept
    {
      return _source->HasEverAnimated();
    }

    // Decoding
    KRYS_NODISCARD bool IsLargeForDecoding() const noexcept
    {
      return _source->IsLargeForDecoding();
    }
    void StopDecodingWorkQueue() noexcept
    {
      _source->StopDecodingWorkQueue();
    }
    void Decode(Func<void(DecodingStatus)> &&decodeCallback) noexcept
    {
      _source->Decode(krys::move(decodeCallback));
    }

    // Current ImageFrame
    KRYS_NODISCARD uint32 CurrentFrameIndex() const noexcept
    {
      return _source->CurrentFrameIndex();
    }
    KRYS_NODISCARD bool CurrentFrameHasAlpha() const noexcept
    {
      return _source->CurrentImageFrame().HasAlpha();
    }
    KRYS_NODISCARD ImageOrientation CurrentFrameOrientation() const noexcept
    {
      return _source->CurrentImageFrame().Orientation();
    }
    KRYS_NODISCARD DecodingOptions CurrentFrameDecodingOptions() const noexcept
    {
      return _source->CurrentImageFrame().GetDecodingOptions();
    }

    // Primary & current NativeImage
    KRYS_NODISCARD RefPtr<NativeImage> PrimaryNativeImage() noexcept
    {
      return _source->PrimaryNativeImage();
    }
    KRYS_NODISCARD RefPtr<NativeImage> GetNativeImage() noexcept final
    {
      return PrimaryNativeImage();
    }
    KRYS_NODISCARD RefPtr<NativeImage> CurrentNativeImage() noexcept final
    {
      return _source->CurrentNativeImage();
    }

    // Image Metadata
    KRYS_NODISCARD FloatSize
      Size(ImageOrientation orientation = ImageOrientation::Orientation::FromImage) const noexcept final
    {
      return FloatSize(_source->Size(orientation));
    }
    KRYS_NODISCARD FloatSize
      SourceSize(ImageOrientation orientation = ImageOrientation::Orientation::FromImage) const noexcept final
    {
      return FloatSize(_source->SourceSize(orientation));
    }
    KRYS_NODISCARD ImageOrientation Orientation() const noexcept final
    {
      return _source->Orientation();
    }
    KRYS_NODISCARD uint32 FrameCount() const noexcept final
    {
      return _source->FrameCount();
    }

    // ImageFrame
    KRYS_NODISCARD seconds FrameDurationAtIndex(uint32 index) const noexcept
    {
      return _source->FrameDurationAtIndex(index);
    }

    // NativeImage
    KRYS_NODISCARD RefPtr<NativeImage> NativeImageAtIndex(uint32 index) noexcept final
    {
      return _source->NativeImageAtIndex(index);
    }

    // Encoded and decoded data
    EncodedDataStatus DataChanged(bool allDataReceived) noexcept final;
    void DestroyDecodedData(bool destroyAll = true) noexcept final;

    // Image Metadata
    KRYS_NODISCARD bool HasDensityCorrectedSize() const noexcept final
    {
      return _source->HasDensityCorrectedSize();
    }
    KRYS_NODISCARD utf8_string FilenameExtension() const noexcept final
    {
      return _source->FilenameExtension();
    }
    KRYS_NODISCARD utf8_string AccessibilityDescription() const noexcept final
    {
      return _source->AccessibilityDescription();
    }
    KRYS_NODISCARD Maybe<IntPoint> HotSpot() const noexcept final
    {
      return _source->HotSpot();
    }

    // Image methods
    KRYS_NODISCARD bool IsBitmapImage() const noexcept final
    {
      return true;
    }
    KRYS_NODISCARD bool IsAnimating() const noexcept final
    {
      return _source->IsAnimating();
    }

    // Current ImageFrame
    KRYS_NODISCARD bool CurrentFrameKnownToBeOpaque() const noexcept final
    {
      return !CurrentFrameHasAlpha();
    }
    KRYS_NODISCARD bool CurrentFrameIsComplete() const noexcept final
    {
      return _source->CurrentImageFrame().IsComplete();
    }

    void DrawPattern(GraphicsContext &, const FloatRect &destinationRect, const FloatRect &tileRect,
                     const AffineTransform &patternTransform, const FloatPoint &phase,
                     const FloatSize &spacing, ImagePaintingOptions = {}) noexcept final;

    void Dump(TextStream &) const noexcept final;

  private:
    BitmapImage(ImageObserver *, AlphaOption, GammaAndColorProfileOption) noexcept;
    explicit BitmapImage(Ref<NativeImage> &&) noexcept;

    // Current NativeImage
    KRYS_NODISCARD RefPtr<NativeImage>
      CurrentPreTransformedNativeImage(ImageOrientation orientation) noexcept final
    {
      return _source->CurrentPreTransformedNativeImage(orientation);
    }

    // Image Metadata
    KRYS_NODISCARD Maybe<Color> SinglePixelSolidColor() const noexcept final
    {
      return _source->SinglePixelSolidColor();
    }

    ImageDrawResult Draw(GraphicsContext &, const FloatRect &destinationRect, const FloatRect &sourceRect,
                         ImagePaintingOptions = {}) noexcept final;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::BitmapImage)
  KRYS_NODISCARD static bool IsType(const krys::boo::Image &image) noexcept
  {
    return image.IsBitmapImage();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
