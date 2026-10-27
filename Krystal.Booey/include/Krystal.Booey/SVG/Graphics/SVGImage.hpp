#pragma once

#include "Krystal.Booey/Graphics/FloatSize.hpp"
#include "Krystal.Booey/Graphics/Image.hpp"
#include "Krystal.Booey/Graphics/IntSize.hpp"
#include "Krystal.Booey/Platform/Timer.hpp"
#include "Krystal.Booey/URL/URL.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/TypeCast.hpp"

namespace krys::boo
{
  class LocalFrameView;
  class Page;
  class RenderBox;
}

namespace krys::boo::dom
{
  class Element;
}

namespace krys::boo::svg
{
  class SVGImageChromeClient;
  class SVGImageForContainer;
  class SVGSvgElement;

  /// @brief An image drawn from an SVG file: the file is a document in a page of its own, laid out at the
  /// size it's drawn at.
  class SVGImage final : public Image
  {
    friend class SVGImageChromeClient;
    friend class SVGImageForContainer;

  private:
    RefPtr<Page> _page;
    Timer _startAnimationTimer;
    FloatSize _intrinsicSize;

  public:
    KRYS_NODISCARD static Ref<SVGImage> Create(ImageObserver *observer) noexcept
    {
      return AdoptRef(*new SVGImage(observer));
    }

    ~SVGImage() noexcept override;

    KRYS_NODISCARD RenderBox *EmbeddedContentBox() const noexcept;
    KRYS_NODISCARD LocalFrameView *FrameView() const noexcept;

    KRYS_NODISCARD bool IsSVGImage() const noexcept final
    {
      return true;
    }

    KRYS_NODISCARD FloatSize
      Size(ImageOrientation = ImageOrientation::Orientation::FromImage) const noexcept final
    {
      return _intrinsicSize;
    }

    KRYS_NODISCARD bool RenderingTaintsOrigin() const noexcept final;

    KRYS_NODISCARD bool HasRelativeWidth() const noexcept final;
    KRYS_NODISCARD bool HasRelativeHeight() const noexcept final;

    // Start the animation from the beginning.
    void StartAnimation() noexcept final;
    // Resume the animation from where it was last stopped.
    void ResumeAnimation() noexcept;
    void StopAnimation() noexcept final;
    void ResetAnimation() noexcept final;
    KRYS_NODISCARD bool IsAnimating() const noexcept final;

    void ScheduleStartAnimation() noexcept;

    KRYS_NODISCARD Page *InternalPage() noexcept
    {
      return _page.get();
    }
    KRYS_NODISCARD RefPtr<SVGSvgElement> RootElement() const noexcept;

  private:
    explicit SVGImage(ImageObserver *) noexcept;

    void StartAnimationTimerFired() noexcept;

    KRYS_NODISCARD utf8_string FilenameExtension() const noexcept final;

    void SetContainerSize(const FloatSize &) noexcept final;
    KRYS_NODISCARD IntSize ContainerSize() const noexcept;
    KRYS_NODISCARD bool UsesContainerSize() const noexcept final
    {
      return true;
    }
    void ComputeIntrinsicDimensions(float &intrinsicWidth, float &intrinsicHeight,
                                    FloatSize &intrinsicRatio) noexcept final;

    EncodedDataStatus DataChanged(bool allDataReceived) noexcept final;

    // FIXME: Implement this to be less conservative.
    KRYS_NODISCARD bool CurrentFrameKnownToBeOpaque() const noexcept final
    {
      return false;
    }
    KRYS_NODISCARD bool CurrentFrameIsComplete() const noexcept final
    {
      return static_cast<bool>(_page);
    }

    ImageDrawResult Draw(GraphicsContext &, const FloatRect &destination, const FloatRect &source,
                         ImagePaintingOptions = {}) noexcept final;
    ImageDrawResult DrawForContainer(GraphicsContext &, FloatSize containerSize, float containerZoom,
                                     const url::URL &initialFragmentURL, const FloatRect &dstRect,
                                     const FloatRect &srcRect, ImagePaintingOptions = {}) noexcept;
    void DrawPatternForContainer(GraphicsContext &, const FloatSize &containerSize, float containerZoom,
                                 const url::URL &initialFragmentURL, const FloatRect &srcRect,
                                 const AffineTransform &, const FloatPoint &phase, const FloatSize &spacing,
                                 const FloatRect &, ImagePaintingOptions = {}) noexcept;
  };

  /// @brief Whether @p element is in an SVG image's document.
  KRYS_NODISCARD bool IsInSVGImage(const dom::Element *element) noexcept;
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::svg::SVGImage)
  KRYS_NODISCARD static bool IsType(const krys::boo::Image &image) noexcept
  {
    return image.IsSVGImage();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
