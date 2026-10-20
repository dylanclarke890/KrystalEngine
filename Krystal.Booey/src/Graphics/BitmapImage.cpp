#include "Krystal.Booey/Graphics/BitmapImage.hpp"
#include "Krystal.Booey/Graphics/BitmapImageSource.hpp"
#include "Krystal.Booey/Graphics/GraphicsContext.hpp"
#include "Krystal.Booey/Graphics/ImageObserver.hpp"
#include "Krystal.Booey/Graphics/NativeImageSource.hpp"

namespace krys::boo
{
  Ref<BitmapImage> BitmapImage::Create(ImageObserver *observer, AlphaOption alphaOption,
                                       GammaAndColorProfileOption gammaAndColorProfileOption) noexcept
  {
    return AdoptRef(*new BitmapImage(observer, alphaOption, gammaAndColorProfileOption));
  }

  Ref<BitmapImage> BitmapImage::Create(Ref<NativeImage> &&nativeImage) noexcept
  {
    return AdoptRef(*new BitmapImage(krys::move(nativeImage)));
  }

  RefPtr<BitmapImage> BitmapImage::Create(RefPtr<NativeImage> &&nativeImage) noexcept
  {
    if (!nativeImage)
    {
      return nullptr;
    }
    return AdoptRef(*new BitmapImage(Ref<NativeImage>(krys::move(nativeImage))));
  }

  RefPtr<BitmapImage> BitmapImage::Create(PlatformImagePtr &&platformImage) noexcept
  {
    return Create(NativeImage::Create(krys::move(platformImage)));
  }

  BitmapImage::BitmapImage(ImageObserver *observer, AlphaOption alphaOption,
                           GammaAndColorProfileOption gammaAndColorProfileOption) noexcept
      : Image(observer), _source(BitmapImageSource::Create(*this, alphaOption, gammaAndColorProfileOption))
  {
  }

  BitmapImage::BitmapImage(Ref<NativeImage> &&image) noexcept
      : _source(NativeImageSource::Create(krys::move(image)))
  {
  }

  EncodedDataStatus BitmapImage::DataChanged(bool allDataReceived) noexcept
  {
    return _source->DataChanged(Data(), allDataReceived);
  }

  void BitmapImage::DestroyDecodedData(bool destroyAll) noexcept
  {
    _source->DestroyDecodedData(destroyAll);
  }

  ImageDrawResult BitmapImage::Draw(GraphicsContext &context, const FloatRect &destinationRect,
                                    const FloatRect &sourceRect, ImagePaintingOptions options) noexcept
  {
    if (destinationRect.IsEmpty() || sourceRect.IsEmpty())
    {
      return ImageDrawResult::DidNothing;
    }

    auto size = FloatSize(_source->Size(ImageOrientation::Orientation::None));
    auto sourceSize = FloatSize(_source->SourceSize(ImageOrientation::Orientation::None));

    // adjustedSourceRect is in the coordinates of densityCorrectedSize, so map it to the sourceSize.
    auto adjustedSourceRect = sourceRect;
    if (sourceSize != size)
    {
      adjustedSourceRect.Scale(sourceSize / size);
    }

    auto scaleFactorForDrawing = context.ScaleFactorForDrawing(destinationRect, adjustedSourceRect);
    auto sizeForDrawing = ExpandedIntSize(sourceSize * scaleFactorForDrawing);
    auto subsamplingLevel = _source->SubsamplingLevelForScaleFactor(context, scaleFactorForDrawing,
                                                                    options.GetAllowImageSubsampling());

    auto nativeImageOrError =
      _source->CurrentNativeImageForDrawing(subsamplingLevel, {options.GetDecodingMode(), sizeForDrawing});

    if (!nativeImageOrError)
    {
      // The decoder has not returned a frame. Fill the image rectangle with a debugging color to show what
      // has happened.
      if (options.GetShowDebugBackground() == ShowDebugBackground::Yes)
      {
        if (nativeImageOrError.error() == DecodingStatus::Decoding)
        {
          FillWithSolidColor(context, destinationRect, SRGBA<uint8> {255, 255, 0, 128},
                             options.GetCompositeOperator());
        }
        else if (nativeImageOrError.error() == DecodingStatus::Invalid)
        {
          FillWithSolidColor(context, destinationRect, SRGBA<uint8> {255, 0, 0, 128},
                             options.GetCompositeOperator());
        }
      }
      return nativeImageOrError.error() == DecodingStatus::Decoding ? ImageDrawResult::DidRequestDecoding
                                                                    : ImageDrawResult::DidNothing;
    }

    Ref<NativeImage> nativeImage = krys::move(*nativeImageOrError);
    if (auto color = nativeImage->SinglePixelSolidColor())
    {
      FillWithSolidColor(context, destinationRect, *color, options.GetCompositeOperator());
    }
    else
    {
      // adjustedSourceRect is in the coordinates of the unsubsampled image, so map it to the subsampled
      // image.
      auto imageSize = FloatSize(nativeImage->Size());
      if (imageSize != sourceSize)
      {
        adjustedSourceRect.Scale(imageSize / sourceSize);
      }

      auto orientation = options.Orientation();
      if (orientation == ImageOrientation::Orientation::FromImage)
      {
        orientation = CurrentFrameOrientation();
      }

      context.DrawNativeImage(*nativeImage, destinationRect, adjustedSourceRect, {options, orientation});
    }

    if (auto observer = GetImageObserver())
    {
      observer->DidDraw(*this);
    }

    return ImageDrawResult::DidDraw;
  }

  void BitmapImage::DrawPattern(GraphicsContext &context, const FloatRect &destinationRect,
                                const FloatRect &tileRect, const AffineTransform &transform,
                                const FloatPoint &phase, const FloatSize &spacing,
                                ImagePaintingOptions options) noexcept
  {
    if (tileRect.IsEmpty())
    {
      return;
    }

    // FIXME: Luminance masks (drawLuminanceMask, convertToLuminanceMask) come with masks.
    Image::DrawPattern(context, destinationRect, tileRect, transform, phase, spacing,
                       {options, ImageOrientation(ImageOrientation::Orientation::FromImage)});
  }

  void BitmapImage::Dump(TextStream &ts) const noexcept
  {
    Image::Dump(ts);
    _source->Dump(ts);
  }
}
