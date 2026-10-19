#pragma once

#include "Krystal.Booey/Graphics/Color/Color.hpp"
#include "Krystal.Booey/Graphics/FloatRect.hpp"
#include "Krystal.Booey/Graphics/ImagePaintingOptions.hpp"
#include "Krystal.Booey/Graphics/Transforms/AffineTransform.hpp"
#include "Krystal.Core/CanMakeWeakPtr.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Types/Maybe.hpp"

namespace krys
{
  class TextStream;
}

namespace krys::boo
{
  class GraphicsContext;

  /// @brief Something drawable as an image: generated images (gradients) for now.
  /// FIXME: Bitmap, SVG and PDF images (their data, decoding, animation, observers and native images) and
  /// tiling by `TileRule` (for `border-image`) come with images.
  class Image : public RefCounted<Image>, public CanMakeWeakPtr<Image>
  {
    friend class GraphicsContext;

  public:
    enum TileRule
    {
      StretchTile,
      RoundTile,
      SpaceTile,
      RepeatTile
    };

    virtual ~Image() noexcept;

    KRYS_NODISCARD virtual bool IsBitmapImage() const noexcept
    {
      return false;
    }
    KRYS_NODISCARD virtual bool IsGeneratedImage() const noexcept
    {
      return false;
    }
    KRYS_NODISCARD virtual bool IsGradientImage() const noexcept
    {
      return false;
    }

    KRYS_NODISCARD virtual bool CurrentFrameKnownToBeOpaque() const noexcept = 0;
    KRYS_NODISCARD virtual bool CurrentFrameIsComplete() const noexcept = 0;

    /// @brief An empty image, drawn as nothing.
    KRYS_NODISCARD static Image &NullImage() noexcept;
    KRYS_NODISCARD bool IsNull() const noexcept
    {
      return Size().IsEmpty();
    }

    virtual void SetContainerSize(const FloatSize &) noexcept
    {
    }
    KRYS_NODISCARD virtual bool UsesContainerSize() const noexcept
    {
      return false;
    }
    KRYS_NODISCARD virtual bool HasRelativeWidth() const noexcept
    {
      return false;
    }
    KRYS_NODISCARD virtual bool HasRelativeHeight() const noexcept
    {
      return false;
    }
    virtual void ComputeIntrinsicDimensions(float &intrinsicWidth, float &intrinsicHeight,
                                            FloatSize &intrinsicRatio) noexcept;

    KRYS_NODISCARD virtual FloatSize
      Size(ImageOrientation = ImageOrientation::Orientation::FromImage) const noexcept = 0;
    KRYS_NODISCARD virtual FloatSize
      SourceSize(ImageOrientation = ImageOrientation::Orientation::FromImage) const noexcept;
    KRYS_NODISCARD FloatRect Rect() const noexcept
    {
      return FloatRect(FloatPoint(), Size());
    }
    KRYS_NODISCARD float Width() const noexcept
    {
      return Size().Width();
    }
    KRYS_NODISCARD float Height() const noexcept
    {
      return Size().Height();
    }

    KRYS_NODISCARD virtual ImageOrientation Orientation() const noexcept
    {
      return ImageOrientation::Orientation::FromImage;
    }

    virtual void DrawPattern(GraphicsContext &, const FloatRect &destRect, const FloatRect &srcRect,
                             const AffineTransform &patternTransform, const FloatPoint &phase,
                             const FloatSize &spacing, ImagePaintingOptions = {}) noexcept;

    virtual void Dump(TextStream &) const noexcept;

  protected:
    Image() noexcept;

    static void FillWithSolidColor(GraphicsContext &, const FloatRect &dstRect, const Color &,
                                   CompositeOperator) noexcept;

    virtual ImageDrawResult Draw(GraphicsContext &, const FloatRect &dstRect, const FloatRect &srcRect,
                                 ImagePaintingOptions = {}) noexcept = 0;
    ImageDrawResult DrawTiled(GraphicsContext &, const FloatRect &dstRect, const FloatPoint &srcPoint,
                              const FloatSize &tileSize, const FloatSize &spacing,
                              ImagePaintingOptions = {}) noexcept;

    // Supporting tiled drawing
    KRYS_NODISCARD virtual Maybe<Color> SinglePixelSolidColor() const noexcept;
  };

  TextStream &operator<<(TextStream &, const Image &) noexcept;
}
