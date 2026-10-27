#pragma once

#include "Krystal.Booey/CSS/Values/ImageValue.hpp"
#include "Krystal.Booey/Loader/Cache/CachedResourceHandle.hpp"
#include "Krystal.Booey/Rendering/Style/StyleImage.hpp"

namespace krys::boo
{
  namespace dom
  {
    class DocumentOrShadowRoot;
  }

  class LegacyRenderSVGResourceContainer;

  /// @brief A `url()` image, or an SVG resource it refers to by its fragment (a `<mask>`).
  class StyleCachedImage final : public StyleImage
  {
  private:
    style::URL _url;
    const Ref<css::ImageValue> _cssValue;
    bool _isPending {true};
    mutable float _scaleFactor {1};
    CachedResourceHandle<CachedImage> _cachedImage;
    mutable Maybe<bool> _isRenderSVGResource;
    FloatSize _containerSize;

  public:
    KRYS_NODISCARD static Ref<StyleCachedImage> Create(style::URL &&, Ref<css::ImageValue> &&,
                                                       float scaleFactor = 1) noexcept;
    KRYS_NODISCARD static Ref<StyleCachedImage> Create(const style::URL &, const Ref<css::ImageValue> &,
                                                       float scaleFactor = 1) noexcept;
    KRYS_NODISCARD static Ref<StyleCachedImage> CopyOverridingScaleFactor(StyleCachedImage &,
                                                                          float scaleFactor) noexcept;
    ~StyleCachedImage() noexcept override;

    KRYS_NODISCARD bool Equals(const StyleImage &) const noexcept final;
    KRYS_NODISCARD bool Equals(const StyleCachedImage &) const noexcept;

    KRYS_NODISCARD CachedImage *GetCachedImage() const noexcept final;

    KRYS_NODISCARD WrappedImagePtr Data() const noexcept final;

    KRYS_NODISCARD Ref<css::Value> ComputedStyleValue(const RenderStyle &) const noexcept final;

    KRYS_NODISCARD bool CanRender(const RenderElement *, float multiplier) const noexcept final;
    KRYS_NODISCARD bool IsPending() const noexcept final;
    void Load(CachedResourceLoader &, const ResourceLoaderOptions &) noexcept final;
    KRYS_NODISCARD bool IsLoaded(const RenderElement *) const noexcept final;
    KRYS_NODISCARD bool ErrorOccurred() const noexcept final;
    KRYS_NODISCARD FloatSize ImageSize(const RenderElement *, float multiplier) const noexcept final;
    KRYS_NODISCARD bool ImageHasRelativeWidth() const noexcept final;
    KRYS_NODISCARD bool ImageHasRelativeHeight() const noexcept final;
    void ComputeIntrinsicDimensions(const RenderElement *, float &intrinsicWidth, float &intrinsicHeight,
                                    FloatSize &intrinsicRatio) noexcept final;
    KRYS_NODISCARD bool UsesImageContainerSize() const noexcept final;
    void SetContainerContextForRenderer(const RenderElement &, const FloatSize &, float) noexcept final;
    void AddClient(RenderElement &) noexcept final;
    void RemoveClient(RenderElement &) noexcept final;
    KRYS_NODISCARD bool HasClient(RenderElement &) const noexcept final;
    KRYS_NODISCARD bool HasImage() const noexcept final;
    KRYS_NODISCARD RefPtr<Image> GetImage(const RenderElement *, const FloatSize &, const GraphicsContext &,
                                          bool isForFirstLine) const noexcept final;
    KRYS_NODISCARD float ImageScaleFactor() const noexcept final;
    KRYS_NODISCARD bool KnownToBeOpaque(const RenderElement &) const noexcept final;
    KRYS_NODISCARD bool UsesDataProtocol() const noexcept final;

    KRYS_NODISCARD style::URL Url() const noexcept final;

  private:
    StyleCachedImage(style::URL &&, Ref<css::ImageValue> &&, float) noexcept;

    KRYS_NODISCARD LegacyRenderSVGResourceContainer *
      UncheckedRenderSVGResource(dom::DocumentOrShadowRoot &,
                                 const dom::DOMStringAtom &fragment) const noexcept;
    KRYS_NODISCARD LegacyRenderSVGResourceContainer *
      UncheckedRenderSVGResource(const RenderElement *) const noexcept;
    KRYS_NODISCARD LegacyRenderSVGResourceContainer *
      LegacyRenderSVGResource(const RenderElement *) const noexcept;
    // FIXME: The layer-based engine's resource (renderSVGResource) isn't ported.
    KRYS_NODISCARD bool IsRenderSVGResource(const RenderElement *) const noexcept;
    StyleCachedImage(const style::URL &, const Ref<css::ImageValue> &, float) noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_TRAITS_STYLE_IMAGE(StyleCachedImage, IsCachedImage)
