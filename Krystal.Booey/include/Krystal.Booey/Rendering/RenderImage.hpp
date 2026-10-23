#pragma once

#include "Krystal.Booey/Graphics/IntSize.hpp"
#include "Krystal.Booey/Rendering/RenderImageResource.hpp"
#include "Krystal.Booey/Rendering/RenderReplaced.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"

namespace krys::boo::html
{
  class HTMLAreaElement;
  class HTMLMapElement;
}

namespace krys::boo
{
  class StyleImage;
  enum class ImageDrawResult : uint8;

  enum class ImageSizeChangeType : bool
  {
    None,
    ForAltText
  };

  /// @brief The renderer of an image: an `<img>`, or generated content (`content: url(...)`).
  // FIXME: Shadow content (image overlays and controls), selection geometry, the broken image icon, and
  // whether the foreground is known to be opaque.
  class RenderImage : public RenderReplaced
  {
    KRYS_NON_COPY_MOVABLE(RenderImage);
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(RenderImage);

  private:
    // Text to display as long as the image isn't available.
    utf8_string _altText;
    UniquePtr<RenderImageResource> _imageResource;
    bool _needsToSetSizeForAltText {false};
    bool _isGeneratedContent {false};
    float _imageDevicePixelRatio {1};

  public:
    RenderImage(Type type, dom::Element &element, RenderStyle &&style, StyleImage *styleImage = nullptr,
                float imageDevicePixelRatio = 1.0f) noexcept;
    RenderImage(Type type, dom::Document &document, RenderStyle &&style,
                StyleImage *styleImage = nullptr) noexcept;
    ~RenderImage() noexcept override;

    KRYS_NODISCARD RenderImageResource &ImageResource() noexcept
    {
      return *_imageResource;
    }
    KRYS_NODISCARD const RenderImageResource &ImageResource() const noexcept
    {
      return *_imageResource;
    }
    KRYS_NODISCARD CachedImage *GetCachedImage() const noexcept
    {
      return ImageResource().GetCachedImage();
    }

    ImageSizeChangeType SetImageSizeForAltText(CachedImage *newImage = nullptr) noexcept;

    void UpdateAltText() noexcept;

    void SetIsGeneratedContent(bool generated = true) noexcept
    {
      _isGeneratedContent = generated;
    }
    KRYS_NODISCARD bool IsGeneratedContent() const noexcept
    {
      return _isGeneratedContent;
    }

    KRYS_NODISCARD const utf8_string &AltText() const noexcept
    {
      return _altText;
    }
    void SetAltText(utf8_string altText) noexcept
    {
      _altText = krys::move(altText);
    }

    void SetImageDevicePixelRatio(float factor) noexcept;
    KRYS_NODISCARD float ImageDevicePixelRatio() const noexcept
    {
      return _imageDevicePixelRatio;
    }

    KRYS_NODISCARD bool IsShowingMissingOrImageError() const noexcept;
    KRYS_NODISCARD bool IsShowingAltText() const noexcept;

    KRYS_NODISCARD virtual bool ShouldDisplayBrokenImageIcon() const noexcept;

    /// @brief Repaints the focus ring of @p element, an area of the image's map, as it gains or loses focus.
    void AreaElementFocusChanged(html::HTMLAreaElement &element) noexcept;

    /// @brief The image map the image uses, if any.
    KRYS_NODISCARD html::HTMLMapElement *ImageMap() const noexcept;
    KRYS_NODISCARD bool ShouldRespectZeroIntrinsicWidth() const noexcept override;

    KRYS_NODISCARD utf8_stringview RenderName() const noexcept override
    {
      return u8"RenderImage";
    }

  protected:
    void WillBeDestroyed() noexcept override;

    KRYS_NODISCARD bool ShouldInvalidatePreferredWidths() const noexcept final;
    KRYS_NODISCARD FloatSize ComputeIntrinsicSize() const noexcept final;
    KRYS_NODISCARD FloatSize PreferredAspectRatio() const noexcept final;

    void StyleWillChange(style::Difference diff, const RenderStyle &newStyle) noexcept override;
    void StyleDidChange(style::Difference diff, const RenderStyle *oldStyle) noexcept override;

    void ImageChanged(WrappedImagePtr newImage, const IntRect *rect = nullptr) noexcept override;

    ImageDrawResult PaintIntoRect(PaintInfo &paintInfo, const FloatRect &rect) noexcept;
    void Layout() noexcept override;

    void IntrinsicSizeChanged() noexcept override
    {
      ImageChanged(ImageResource().ImagePtr());
    }

  private:
    KRYS_NODISCARD bool CanHaveChildren() const noexcept override
    {
      return false;
    }

    void Paint(PaintInfo &paintInfo, const LayoutPoint &paintOffset) noexcept override;
    void PaintReplaced(PaintInfo &paintInfo, const LayoutPoint &paintOffset) noexcept override;
    void PaintAreaElementFocusRing(PaintInfo &paintInfo, const LayoutPoint &paintOffset) noexcept;

    bool NodeAtPoint(const HitTestRequest &request, HitTestResult &result,
                     const HitTestLocation &locationInContainer, const LayoutPoint &accumulatedOffset,
                     HitTestAction action) noexcept override;
    void PaintIncompleteImageOutline(PaintInfo &paintInfo, LayoutPoint paintOffset,
                                     LayoutUnit borderWidth) const noexcept;

    KRYS_NODISCARD LayoutUnit MinimumReplacedHeight() const noexcept override;

    void NotifyFinished(CachedResource &newImage) noexcept final;

    KRYS_NODISCARD IntSize ImageSizeForError(CachedImage *newImage) const noexcept;
    void RepaintOrMarkForLayout(ImageSizeChangeType imageSizeChange, const IntRect *rect = nullptr) noexcept;
    void UpdateIntrinsicSizeIfNeeded(const LayoutSize &newSize) noexcept;
    // Update the size of the image to be rendered. Object-fit may cause this to be different from the CSS
    // box's content rect.
    void UpdateInnerContentRect() noexcept;

    KRYS_NODISCARD LayoutUnit
      ComputeReplacedLogicalWidth(ShouldComputePreferred shouldComputePreferred =
                                    ShouldComputePreferred::ComputeActual) const noexcept override;
    KRYS_NODISCARD LayoutUnit
      ComputeReplacedLogicalHeight(Maybe<LayoutUnit> estimatedUsedWidth = null) const noexcept override;

    KRYS_NODISCARD bool ShouldCollapseToEmpty() const noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::RenderImage)
  KRYS_NODISCARD static bool IsType(const krys::boo::RenderObject &renderer) noexcept
  {
    return renderer.IsRenderImage();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
