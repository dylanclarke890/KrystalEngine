#pragma once

#include "Krystal.Booey/Graphics/Color/Color.hpp"
#include "Krystal.Booey/Graphics/FloatRect.hpp"
#include "Krystal.Booey/Graphics/ImageObserver.hpp"
#include "Krystal.Booey/Graphics/ImagePaintingOptions.hpp"
#include "Krystal.Booey/Graphics/IntPoint.hpp"
#include "Krystal.Booey/Graphics/Transforms/AffineTransform.hpp"
#include "Krystal.Booey/Platform/SharedBuffer.hpp"
#include "Krystal.Core/CanMakeWeakPtr.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"

namespace krys
{
  class TextStream;
}

namespace krys::boo
{
  class GraphicsContext;
  class NativeImage;

  /// @brief Something drawable as an image: a bitmap image, an SVG image or a generated image (a gradient).
  /// FIXME: PDF images, the image adapter, animation controls and the asynchronous animation start (G7d).
  class Image : public RefCounted<Image>, public CanMakeWeakPtr<Image>
  {
    friend class GraphicsContext;

  private:
    RefPtr<SharedBuffer> _encodedImageData;
    WeakPtr<ImageObserver> _imageObserver;

  public:
    enum TileRule
    {
      StretchTile,
      RoundTile,
      SpaceTile,
      RepeatTile
    };

    virtual ~Image() noexcept;

    /// @brief The image for @p observer's resource: an SVG image for SVG, a bitmap image for the rest.
    KRYS_NODISCARD static RefPtr<Image> Create(ImageObserver &observer) noexcept;

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
    KRYS_NODISCARD virtual bool IsSVGImage() const noexcept
    {
      return false;
    }
    KRYS_NODISCARD virtual bool IsSVGImageForContainer() const noexcept
    {
      return false;
    }
    KRYS_NODISCARD bool DrawsSVGImage() const noexcept
    {
      return IsSVGImage() || IsSVGImageForContainer();
    }

    KRYS_NODISCARD virtual uint32 FrameCount() const noexcept
    {
      return 1;
    }
    KRYS_NODISCARD virtual bool CurrentFrameKnownToBeOpaque() const noexcept = 0;
    KRYS_NODISCARD virtual bool CurrentFrameIsComplete() const noexcept = 0;
    KRYS_NODISCARD virtual bool IsAnimated() const noexcept
    {
      return false;
    }

    // Derived classes should override this if their rendering could leak
    // cross-origin data (outside of the resource itself, which undergoes
    // a CORS cross-origin check).
    KRYS_NODISCARD virtual bool RenderingTaintsOrigin() const noexcept
    {
      return false;
    }

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
    KRYS_NODISCARD virtual bool HasDensityCorrectedSize() const noexcept
    {
      return false;
    }
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

    KRYS_NODISCARD virtual Maybe<IntPoint> HotSpot() const noexcept
    {
      return null;
    }

    KRYS_NODISCARD virtual ImageOrientation Orientation() const noexcept
    {
      return ImageOrientation::Orientation::FromImage;
    }

    /// @brief Replaces the encoded data and tells the image it changed.
    EncodedDataStatus SetData(RefPtr<SharedBuffer> &&data, bool allDataReceived) noexcept;
    virtual EncodedDataStatus DataChanged(bool /*allDataReceived*/) noexcept
    {
      return EncodedDataStatus::Unknown;
    }

    /// @brief The file extension the data's format uses; empty if unknown.
    KRYS_NODISCARD virtual utf8_string FilenameExtension() const noexcept
    {
      return {};
    }
    /// @brief A description of the image the data carries; empty if unknown.
    KRYS_NODISCARD virtual utf8_string AccessibilityDescription() const noexcept
    {
      return {};
    }

    virtual void DestroyDecodedData(bool /*destroyAll*/ = true) noexcept
    {
    }

    KRYS_NODISCARD SharedBuffer *Data() noexcept
    {
      return _encodedImageData.get();
    }
    KRYS_NODISCARD const SharedBuffer *Data() const noexcept
    {
      return _encodedImageData.get();
    }

    // Animation begins whenever someone draws the image, so startAnimation() is not normally called.
    // It will automatically pause once all observers no longer want to render the image anywhere.
    virtual void StartAnimation() noexcept
    {
    }
    virtual void StopAnimation() noexcept
    {
    }
    virtual void ResetAnimation() noexcept
    {
    }
    KRYS_NODISCARD virtual bool IsAnimating() const noexcept
    {
      return false;
    }

    // Typically the CachedImage that owns us.
    KRYS_NODISCARD RefPtr<ImageObserver> GetImageObserver() const noexcept;
    void SetImageObserver(RefPtr<ImageObserver> &&) noexcept;

    KRYS_NODISCARD url::URL SourceURL() const noexcept;
    KRYS_NODISCARD utf8_string MimeType() const noexcept;
    KRYS_NODISCARD int64 ExpectedContentLength() const noexcept;

    KRYS_NODISCARD virtual RefPtr<NativeImage> GetNativeImage() noexcept;
    KRYS_NODISCARD virtual RefPtr<NativeImage> NativeImageAtIndex(uint32) noexcept;
    KRYS_NODISCARD virtual RefPtr<NativeImage> CurrentNativeImage() noexcept;
    KRYS_NODISCARD virtual RefPtr<NativeImage>
      CurrentPreTransformedNativeImage(ImageOrientation = ImageOrientation::Orientation::FromImage) noexcept;

    virtual void DrawPattern(GraphicsContext &, const FloatRect &destRect, const FloatRect &srcRect,
                             const AffineTransform &patternTransform, const FloatPoint &phase,
                             const FloatSize &spacing, ImagePaintingOptions = {}) noexcept;

    virtual void Dump(TextStream &) const noexcept;

  protected:
    explicit Image(ImageObserver * = nullptr) noexcept;

    static void FillWithSolidColor(GraphicsContext &, const FloatRect &dstRect, const Color &,
                                   CompositeOperator) noexcept;

    virtual ImageDrawResult Draw(GraphicsContext &, const FloatRect &dstRect, const FloatRect &srcRect,
                                 ImagePaintingOptions = {}) noexcept = 0;
    ImageDrawResult DrawTiled(GraphicsContext &, const FloatRect &dstRect, const FloatPoint &srcPoint,
                              const FloatSize &tileSize, const FloatSize &spacing,
                              ImagePaintingOptions = {}) noexcept;
    ImageDrawResult DrawTiled(GraphicsContext &, const FloatRect &dstRect, const FloatRect &srcRect,
                              const FloatSize &tileScaleFactor, TileRule hRule, TileRule vRule,
                              ImagePaintingOptions = {}) noexcept;

    // Supporting tiled drawing
    KRYS_NODISCARD virtual Maybe<Color> SinglePixelSolidColor() const noexcept;
  };

  TextStream &operator<<(TextStream &, const Image &) noexcept;
}
