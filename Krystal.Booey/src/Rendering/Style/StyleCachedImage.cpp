#include "Krystal.Booey/Rendering/Style/StyleCachedImage.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/Loader/Cache/CachedImage.hpp"
#include "Krystal.Booey/Loader/Cache/CachedResourceLoader.hpp"
#include "Krystal.Booey/Rendering/ReferencedSVGResources.hpp"
#include "Krystal.Booey/Rendering/RenderElement.hpp"
#include "Krystal.Booey/Style/Seam.hpp"
#include "Krystal.Booey/SVG/Graphics/SVGImage.hpp"
#include "Krystal.Booey/SVG/Graphics/SVGResourceImage.hpp"
#include "Krystal.Booey/SVG/SVGSvgElement.hpp"
#include "Krystal.Booey/SVG/SVGURIReference.hpp"

namespace krys::boo
{
  Ref<StyleCachedImage> StyleCachedImage::Create(style::URL &&url, Ref<css::ImageValue> &&cssValue,
                                                 float scaleFactor) noexcept
  {
    return AdoptRef(*new StyleCachedImage(krys::move(url), krys::move(cssValue), scaleFactor));
  }

  Ref<StyleCachedImage> StyleCachedImage::Create(const style::URL &url, const Ref<css::ImageValue> &cssValue,
                                                 float scaleFactor) noexcept
  {
    return AdoptRef(*new StyleCachedImage(url, cssValue, scaleFactor));
  }

  Ref<StyleCachedImage> StyleCachedImage::CopyOverridingScaleFactor(StyleCachedImage &other,
                                                                    float scaleFactor) noexcept
  {
    if (other._scaleFactor == scaleFactor)
    {
      return ShareRef(other);
    }
    return StyleCachedImage::Create(other._url, other._cssValue, scaleFactor);
  }

  StyleCachedImage::StyleCachedImage(style::URL &&url, Ref<css::ImageValue> &&cssValue,
                                     float scaleFactor) noexcept
      : StyleImage {Type::CachedImage}, _url {krys::move(url)}, _cssValue {krys::move(cssValue)},
        _scaleFactor {scaleFactor}
  {
    _cachedImage = _cssValue->GetCachedImage();
    if (_cachedImage)
    {
      _isPending = false;
    }
  }

  StyleCachedImage::StyleCachedImage(const style::URL &url, const Ref<css::ImageValue> &cssValue,
                                     float scaleFactor) noexcept
      : StyleImage {Type::CachedImage}, _url {url}, _cssValue {cssValue}, _scaleFactor {scaleFactor}
  {
    _cachedImage = _cssValue->GetCachedImage();
    if (_cachedImage)
    {
      _isPending = false;
    }
  }

  StyleCachedImage::~StyleCachedImage() noexcept = default;

  bool StyleCachedImage::Equals(const StyleImage &other) const noexcept
  {
    const auto *otherCachedImage = DynamicDowncast<StyleCachedImage>(other);
    return otherCachedImage && Equals(*otherCachedImage);
  }

  bool StyleCachedImage::Equals(const StyleCachedImage &other) const noexcept
  {
    if (&other == this)
    {
      return true;
    }
    if (_scaleFactor != other._scaleFactor)
    {
      return false;
    }
    if (_cssValue.get() == other._cssValue.get() || _cssValue->Equals(*other._cssValue))
    {
      return true;
    }
    if (_cachedImage && _cachedImage == other._cachedImage)
    {
      return true;
    }
    return false;
  }

  style::URL StyleCachedImage::Url() const noexcept
  {
    return _url;
  }

  LegacyRenderSVGResourceContainer *
    StyleCachedImage::UncheckedRenderSVGResource(dom::DocumentOrShadowRoot &treeScope,
                                                 const dom::DOMStringAtom &fragment) const noexcept
  {
    auto *renderSVGResource = ReferencedSVGResources::ReferencedRenderResource(treeScope, fragment);
    _isRenderSVGResource = renderSVGResource != nullptr;
    return renderSVGResource;
  }

  LegacyRenderSVGResourceContainer *
    StyleCachedImage::UncheckedRenderSVGResource(const RenderElement *renderer) const noexcept
  {
    if (!renderer)
    {
      return nullptr;
    }

    if (_url.Resolved.String().find(u8'#') == utf8_string::npos)
    {
      _isRenderSVGResource = false;
      return nullptr;
    }

    if (!_cachedImage)
    {
      auto fragmentIdentifier =
        svg::SVGURIReference::FragmentIdentifierFromIRIString(_url, renderer->GetDocument());
      return UncheckedRenderSVGResource(renderer->TreeScopeForSVGReferences(), fragmentIdentifier);
    }

    auto *image = DynamicDowncast<svg::SVGImage>(_cachedImage->GetImage());
    if (!image)
    {
      return nullptr;
    }

    auto rootElement = image->RootElement();
    if (!rootElement)
    {
      return nullptr;
    }

    return UncheckedRenderSVGResource(rootElement->TreeScopeForSVGReferences(),
                                      dom::DOMStringAtom(_url.Resolved.FragmentIdentifier()));
  }

  LegacyRenderSVGResourceContainer *
    StyleCachedImage::LegacyRenderSVGResource(const RenderElement *renderer) const noexcept
  {
    if (_isRenderSVGResource && !*_isRenderSVGResource)
    {
      return nullptr;
    }
    return UncheckedRenderSVGResource(renderer);
  }

  bool StyleCachedImage::IsRenderSVGResource(const RenderElement *renderer) const noexcept
  {
    return LegacyRenderSVGResource(renderer);
  }

  void StyleCachedImage::Load(CachedResourceLoader &loader, const ResourceLoaderOptions &options) noexcept
  {
    krys_debug_assert(_isPending);
    _isPending = false;
    _cachedImage = _cssValue->LoadImage(loader, options);
  }

  WrappedImagePtr StyleCachedImage::Data() const noexcept
  {
    return _cachedImage.get();
  }

  CachedImage *StyleCachedImage::GetCachedImage() const noexcept
  {
    return _cachedImage.get();
  }

  Ref<css::Value> StyleCachedImage::ComputedStyleValue(const RenderStyle &style) const noexcept
  {
    return _cssValue->CopyForComputedStyle(style::ConvertToCSS(_url, style));
  }

  bool StyleCachedImage::CanRender(const RenderElement *renderer, float multiplier) const noexcept
  {
    if (IsRenderSVGResource(renderer))
    {
      return true;
    }
    if (!_cachedImage)
    {
      return false;
    }
    return _cachedImage->CanRender(renderer, multiplier);
  }

  bool StyleCachedImage::IsPending() const noexcept
  {
    return _isPending;
  }

  bool StyleCachedImage::IsLoaded(const RenderElement *renderer) const noexcept
  {
    if (IsRenderSVGResource(renderer))
    {
      return true;
    }
    if (!_cachedImage)
    {
      return false;
    }
    return _cachedImage->IsLoaded();
  }

  bool StyleCachedImage::ErrorOccurred() const noexcept
  {
    if (!_cachedImage)
    {
      return false;
    }
    return _cachedImage->ErrorOccurred();
  }

  FloatSize StyleCachedImage::ImageSize(const RenderElement *renderer, float multiplier) const noexcept
  {
    if (IsRenderSVGResource(renderer))
    {
      return _containerSize;
    }
    if (!_cachedImage)
    {
      return {};
    }
    return FloatSize(_cachedImage->ImageSizeForRenderer(renderer, multiplier)) / _scaleFactor;
  }

  bool StyleCachedImage::ImageHasRelativeWidth() const noexcept
  {
    if (!_cachedImage)
    {
      return false;
    }
    return _cachedImage->ImageHasRelativeWidth();
  }

  bool StyleCachedImage::ImageHasRelativeHeight() const noexcept
  {
    if (!_cachedImage)
    {
      return false;
    }
    return _cachedImage->ImageHasRelativeHeight();
  }

  void StyleCachedImage::ComputeIntrinsicDimensions(const RenderElement *renderer, float &intrinsicWidth,
                                                    float &intrinsicHeight,
                                                    FloatSize &intrinsicRatio) noexcept
  {
    // In case of an SVG resource, we should return the container size.
    if (IsRenderSVGResource(renderer))
    {
      FloatSize size = FloorSizeToDevicePixels(LayoutSize(_containerSize),
                                               renderer ? renderer->GetDocument().DeviceScaleFactor() : 1);
      intrinsicWidth = size.Width();
      intrinsicHeight = size.Height();
      intrinsicRatio = size;
      return;
    }

    if (!_cachedImage)
    {
      return;
    }

    _cachedImage->ComputeIntrinsicDimensions(intrinsicWidth, intrinsicHeight, intrinsicRatio);
  }

  bool StyleCachedImage::UsesImageContainerSize() const noexcept
  {
    if (!_cachedImage)
    {
      return false;
    }
    return _cachedImage->UsesImageContainerSize();
  }

  void StyleCachedImage::SetContainerContextForRenderer(const RenderElement &renderer,
                                                        const FloatSize &containerSize,
                                                        float containerZoom) noexcept
  {
    _containerSize = containerSize;
    if (!_cachedImage)
    {
      return;
    }
    _cachedImage->SetContainerContextForClient(renderer.GetCachedImageClient(), LayoutSize(containerSize),
                                               containerZoom, _url.Resolved);
  }

  void StyleCachedImage::AddClient(RenderElement &renderer) noexcept
  {
    krys_debug_assert(!_isPending);
    if (!_cachedImage)
    {
      return;
    }
    _cachedImage->AddClient(renderer.GetCachedImageClient());
  }

  void StyleCachedImage::RemoveClient(RenderElement &renderer) noexcept
  {
    krys_debug_assert(!_isPending);
    if (!_cachedImage)
    {
      return;
    }
    _cachedImage->RemoveClient(renderer.GetCachedImageClient());
  }

  bool StyleCachedImage::HasClient(RenderElement &renderer) const noexcept
  {
    krys_debug_assert(!_isPending);
    if (!_cachedImage)
    {
      return false;
    }
    return _cachedImage->HasClient(renderer.GetCachedImageClient());
  }

  bool StyleCachedImage::HasImage() const noexcept
  {
    if (!_cachedImage)
    {
      return false;
    }
    return _cachedImage->HasImage();
  }

  RefPtr<Image> StyleCachedImage::GetImage(const RenderElement *renderer, const FloatSize &,
                                           const GraphicsContext &, bool) const noexcept
  {
    krys_debug_assert(!_isPending);

    if (auto *renderSVGResource = LegacyRenderSVGResource(renderer))
    {
      return svg::SVGResourceImage::Create(*renderSVGResource, _url);
    }

    if (!_cachedImage)
    {
      return nullptr;
    }

    return ShareRefPtr(_cachedImage->ImageForRenderer(renderer));
  }

  float StyleCachedImage::ImageScaleFactor() const noexcept
  {
    return _scaleFactor;
  }

  bool StyleCachedImage::KnownToBeOpaque(const RenderElement &renderer) const noexcept
  {
    return _cachedImage && _cachedImage->CurrentFrameKnownToBeOpaque(&renderer);
  }

  bool StyleCachedImage::UsesDataProtocol() const noexcept
  {
    return _url.Resolved.ProtocolIsData();
  }
}
