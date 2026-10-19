#include "Krystal.Booey/Graphics/Image.hpp"
#include "Krystal.Booey/Graphics/GraphicsContext.hpp"
#include "Krystal.Core/Text/TextStream.hpp"
#include "Krystal.Core/Utils/NeverDestroyed.hpp"
#include <cmath>

namespace krys::boo
{
  namespace
  {
    /// @brief An image of no size, which draws nothing.
    class EmptyImage final : public Image
    {
    public:
      KRYS_NODISCARD bool CurrentFrameKnownToBeOpaque() const noexcept override
      {
        return false;
      }
      KRYS_NODISCARD bool CurrentFrameIsComplete() const noexcept override
      {
        return true;
      }
      KRYS_NODISCARD FloatSize Size(ImageOrientation) const noexcept override
      {
        return {};
      }

    protected:
      ImageDrawResult Draw(GraphicsContext &, const FloatRect &, const FloatRect &,
                           ImagePaintingOptions) noexcept override
      {
        return ImageDrawResult::DidNothing;
      }
    };
  }

  Image::Image() noexcept = default;

  Image::~Image() noexcept = default;

  Image &Image::NullImage() noexcept
  {
    static Image &nullImage = *new EmptyImage();
    return nullImage;
  }

  void Image::FillWithSolidColor(GraphicsContext &ctxt, const FloatRect &dstRect, const Color &color,
                                 CompositeOperator op) noexcept
  {
    if (!color.IsVisible())
    {
      return;
    }

    CompositeOperator previousOperator = ctxt.CompositeOperation();
    ctxt.SetCompositeOperation(
      color.IsOpaque() && op == CompositeOperator::SourceOver ? CompositeOperator::Copy : op);
    ctxt.FillRect(dstRect, color);
    ctxt.SetCompositeOperation(previousOperator);
  }

  ImageDrawResult Image::DrawTiled(GraphicsContext &ctxt, const FloatRect &dstRect, const FloatRect &srcRect,
                                   const FloatSize &tileScaleFactor, TileRule hRule, TileRule vRule,
                                   ImagePaintingOptions options) noexcept
  {
    if (auto color = SinglePixelSolidColor())
    {
      FillWithSolidColor(ctxt, dstRect, *color, options.GetCompositeOperator());
      return ImageDrawResult::DidDraw;
    }

    FloatSize tileScale = tileScaleFactor;
    FloatSize spacing;

    // FIXME: These rules follow CSS border-image rules, but they should not be down here in Image.
    bool centerOnGapHorizonally = false;
    bool centerOnGapVertically = false;
    switch (hRule)
    {
      case RoundTile:
      {
        float scaledSourceWidth = srcRect.Width() * tileScale.Width();
        int32 numItems =
          std::max<int32>(static_cast<int32>(std::round(dstRect.Width() / scaledSourceWidth)), 1);
        tileScale.SetWidth(dstRect.Width() / (srcRect.Width() * static_cast<float>(numItems)));
        break;
      }
      case SpaceTile:
      {
        float scaledSourceWidth = srcRect.Width() * tileScale.Width();
        auto numItems = static_cast<int32>(std::floor(dstRect.Width() / scaledSourceWidth));
        if (!numItems)
        {
          return ImageDrawResult::DidNothing;
        }
        spacing.SetWidth((dstRect.Width() - scaledSourceWidth * static_cast<float>(numItems))
                         / static_cast<float>(numItems + 1));
        centerOnGapHorizonally = !(numItems & 1);
        break;
      }
      case StretchTile:
      case RepeatTile:  break;
    }

    switch (vRule)
    {
      case RoundTile:
      {
        float scaledSourceHeight = srcRect.Height() * tileScale.Height();
        int32 numItems =
          std::max<int32>(static_cast<int32>(std::round(dstRect.Height() / scaledSourceHeight)), 1);
        tileScale.SetHeight(dstRect.Height() / (srcRect.Height() * static_cast<float>(numItems)));
        break;
      }
      case SpaceTile:
      {
        float scaledSourceHeight = srcRect.Height() * tileScale.Height();
        auto numItems = static_cast<int32>(std::floor(dstRect.Height() / scaledSourceHeight));
        if (!numItems)
        {
          return ImageDrawResult::DidNothing;
        }
        spacing.SetHeight((dstRect.Height() - scaledSourceHeight * static_cast<float>(numItems))
                          / static_cast<float>(numItems + 1));
        centerOnGapVertically = !(numItems & 1);
        break;
      }
      case StretchTile:
      case RepeatTile:  break;
    }

    AffineTransform patternTransform =
      AffineTransform().ScaleNonUniform(tileScale.Width(), tileScale.Height());

    // We want to construct the phase such that the pattern is centered (when stretch is not
    // set for a particular rule).
    float hPhase = tileScale.Width() * srcRect.X();
    float vPhase = tileScale.Height() * srcRect.Y();
    float scaledTileWidth = tileScale.Width() * srcRect.Width();
    float scaledTileHeight = tileScale.Height() * srcRect.Height();

    if (centerOnGapHorizonally)
    {
      hPhase -= spacing.Width();
    }
    else if (hRule == RepeatTile || hRule == SpaceTile)
    {
      hPhase -= (dstRect.Width() - scaledTileWidth) / 2;
    }

    if (centerOnGapVertically)
    {
      vPhase -= spacing.Height();
    }
    else if (vRule == RepeatTile || vRule == SpaceTile)
    {
      vPhase -= (dstRect.Height() - scaledTileHeight) / 2;
    }

    FloatPoint patternPhase(dstRect.X() - hPhase, dstRect.Y() - vPhase);
    DrawPattern(ctxt, dstRect, srcRect, patternTransform, patternPhase, spacing, options);
    return ImageDrawResult::DidDraw;
  }

  void Image::DrawPattern(GraphicsContext &, const FloatRect &, const FloatRect &, const AffineTransform &,
                          const FloatPoint &, const FloatSize &, ImagePaintingOptions) noexcept
  {
    // FIXME: Bitmap images draw their current frame as a pattern (`drawPattern` with a native image).
  }

  ImageDrawResult Image::DrawTiled(GraphicsContext &ctxt, const FloatRect &destRect,
                                   const FloatPoint &srcPoint, const FloatSize &scaledTileSize,
                                   const FloatSize &spacing, ImagePaintingOptions options) noexcept
  {
    if (auto color = SinglePixelSolidColor())
    {
      FillWithSolidColor(ctxt, destRect, *color, options.GetCompositeOperator());
      return ImageDrawResult::DidDraw;
    }

    FloatSize intrinsicTileSize = Size();
    if (HasRelativeWidth())
    {
      intrinsicTileSize.SetWidth(scaledTileSize.Width());
    }
    if (HasRelativeHeight())
    {
      intrinsicTileSize.SetHeight(scaledTileSize.Height());
    }

    FloatSize scale(scaledTileSize / intrinsicTileSize);

    FloatRect oneTileRect;
    FloatSize actualTileSize = scaledTileSize + spacing;
    oneTileRect.SetX(destRect.X()
                     + std::fmodf(std::fmodf(-srcPoint.X(), actualTileSize.Width()) - actualTileSize.Width(),
                                  actualTileSize.Width()));
    oneTileRect.SetY(
      destRect.Y()
      + std::fmodf(std::fmodf(-srcPoint.Y(), actualTileSize.Height()) - actualTileSize.Height(),
                   actualTileSize.Height()));
    oneTileRect.SetSize(scaledTileSize);

    // Check and see if a single draw of the image can cover the entire area we are supposed to tile.
    if (oneTileRect.Contains(destRect) && !ctxt.DrawLuminanceMask())
    {
      FloatRect visibleSrcRect;
      visibleSrcRect.SetX((destRect.X() - oneTileRect.X()) / scale.Width());
      visibleSrcRect.SetY((destRect.Y() - oneTileRect.Y()) / scale.Height());
      visibleSrcRect.SetWidth(destRect.Width() / scale.Width());
      visibleSrcRect.SetHeight(destRect.Height() / scale.Height());
      return Draw(ctxt, destRect, visibleSrcRect, options);
    }

    // Patterned images and gradients can use lots of memory for caching when the
    // tile size is large (<rdar://problem/4691859>, <rdar://problem/6239505>).
    // Memory consumption depends on the transformed tile size which can get
    // larger than the original tile if user zooms in enough.
    const float maxPatternTilePixels = 2'048 * 2'048;
    FloatRect transformedTileSize = ctxt.GetCTM().MapRect(FloatRect(FloatPoint(), scaledTileSize));
    float transformedTileSizePixels = transformedTileSize.Width() * transformedTileSize.Height();
    FloatRect currentTileRect = oneTileRect;
    if (transformedTileSizePixels > maxPatternTilePixels)
    {
      GraphicsContextStateSaver stateSaver(ctxt);
      ctxt.Clip(destRect);

      currentTileRect.ShiftYEdgeTo(destRect.Y());
      float toY = currentTileRect.Y();
      ImageDrawResult result = ImageDrawResult::DidNothing;
      while (toY < destRect.MaxY())
      {
        currentTileRect.ShiftXEdgeTo(destRect.X());
        float toX = currentTileRect.X();
        while (toX < destRect.MaxX())
        {
          FloatRect toRect(toX, toY, currentTileRect.Width(), currentTileRect.Height());
          FloatRect fromRect(ToFloatPoint(currentTileRect.Location() - oneTileRect.Location()),
                             currentTileRect.Size());
          fromRect.Scale(1 / scale.Width(), 1 / scale.Height());

          result = Draw(ctxt, toRect, fromRect, {options, BlendMode::Normal});
          if (result == ImageDrawResult::DidRequestDecoding)
          {
            return result;
          }
          toX += currentTileRect.Width();
          currentTileRect.ShiftXEdgeTo(oneTileRect.X());
        }
        toY += currentTileRect.Height();
        currentTileRect.ShiftYEdgeTo(oneTileRect.Y());
      }
      return result;
    }

    AffineTransform patternTransform = AffineTransform().ScaleNonUniform(scale.Width(), scale.Height());
    FloatRect tileRect(FloatPoint(), intrinsicTileSize);
    DrawPattern(ctxt, destRect, tileRect, patternTransform, oneTileRect.Location(), spacing, options);
    return ImageDrawResult::DidDraw;
  }

  void Image::ComputeIntrinsicDimensions(float &intrinsicWidth, float &intrinsicHeight,
                                         FloatSize &intrinsicRatio) noexcept
  {
    intrinsicRatio = Size();
    intrinsicWidth = intrinsicRatio.Width();
    intrinsicHeight = intrinsicRatio.Height();
  }

  FloatSize Image::SourceSize(ImageOrientation orientation) const noexcept
  {
    return Size(orientation);
  }

  Maybe<Color> Image::SinglePixelSolidColor() const noexcept
  {
    return std::nullopt;
  }

  void Image::Dump(TextStream &ts) const noexcept
  {
    if (IsNull())
    {
      ts.DumpProperty(u8"is-null-image", true);
    }

    ts.DumpProperty(u8"size", Size());
  }

  TextStream &operator<<(TextStream &ts, const Image &image) noexcept
  {
    TextStream::GroupScope scope(ts);

    if (image.IsBitmapImage())
    {
      ts << "bitmap image";
    }
    else if (image.IsGradientImage())
    {
      ts << "gradient image";
    }

    image.Dump(ts);
    return ts;
  }
}
