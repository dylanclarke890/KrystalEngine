#include "Krystal.Booey/Css/Values/ImageValue.hpp"
#include "Krystal.Booey/Css/Values/Types.hpp"
#include "Krystal.Booey/Dom/Nodes/Document.hpp"
#include "Krystal.Booey/Loader/Cache/CachedImage.hpp"
#include "Krystal.Booey/Loader/Cache/CachedResourceLoader.hpp"
#include "Krystal.Booey/Rendering/Style/StyleCachedImage.hpp"
#include "Krystal.Booey/Style/Builder/BuilderState.hpp"

namespace krys::boo::css
{
  ImageValue::ImageValue(css::Url location, CSSOMStringAtom initiatorType) noexcept
      : Value(ValueType::Image), _location(krys::move(location)),
        _initiatorType(initiatorType.View().empty()
                         ? CSSOMStringAtom(CachedResourceRequestInitiatorTypes::CSS)
                         : krys::move(initiatorType))
  {
  }

  ImageValue::ImageValue(InvalidTag) noexcept : Value(ValueType::Image), _isInvalid(true)
  {
  }

  ImageValue::~ImageValue() noexcept = default;

  Ref<ImageValue> ImageValue::Create(css::Url location, CSSOMStringAtom initiatorType) noexcept
  {
    return AdoptRef(*new ImageValue(krys::move(location), krys::move(initiatorType)));
  }

  Ref<ImageValue> ImageValue::Create(net::Url url, CSSOMStringAtom initiatorType) noexcept
  {
    CSSOMString text {url.String()};
    return Create(css::Url {.Specified = krys::move(text), .Resolved = krys::move(url), .Modifiers = {}},
                  krys::move(initiatorType));
  }

  Ref<ImageValue> ImageValue::CreateInvalid() noexcept
  {
    return AdoptRef(*new ImageValue(InvalidTag {}));
  }

  Ref<ImageValue> ImageValue::CopyWithLocation(css::Url location) const noexcept
  {
    auto copy = Create(krys::move(location), _initiatorType);
    copy->_cachedImage = _cachedImage;
    copy->_didRequestImage = _didRequestImage;
    copy->_unresolvedValue = ShareForLoading();
    return copy;
  }

  Ref<ImageValue> ImageValue::CopyForComputedStyle(const css::Url &resolvedUrl) const noexcept
  {
    return resolvedUrl == _location ? ShareForLoading() : CopyWithLocation(resolvedUrl);
  }

  CachedImage *ImageValue::LoadImage(CachedResourceLoader &loader,
                                     const ResourceLoaderOptions &options) noexcept
  {
    if (_didRequestImage)
    {
      return _cachedImage.get();
    }

    krys_debug_assert(loader.GetDocument());
    ResourceLoaderOptions loadOptions = options;
    ApplyModifiersToLoaderOptions(_location.Modifiers, loadOptions);

    CachedResourceRequest request(net::ResourceRequest(_location.Resolved), loadOptions);
    request.SetInitiatorType(_initiatorType.View());
    auto requested = loader.RequestImage(krys::move(request));
    _cachedImage = requested ? *requested : CachedResourceHandle<CachedImage>();
    _didRequestImage = true;

    // The values this one was copied from share the request, so none of them makes another.
    for (auto *original = _unresolvedValue.get(); original; original = original->_unresolvedValue.get())
    {
      original->_cachedImage = _cachedImage;
      original->_didRequestImage = true;
    }
    return _cachedImage.get();
  }

  CachedImage *ImageValue::GetCachedImage() const noexcept
  {
    return _cachedImage.get();
  }

  RefPtr<StyleImage> ImageValue::CreateStyleImage(const style::BuilderState &state) const noexcept
  {
    auto styleLocation = style::ConvertToStyle(_location, state);
    if (styleLocation.Resolved == _location.Resolved)
    {
      return StyleCachedImage::Create(krys::move(styleLocation), ShareForLoading());
    }

    // FIXME: The URL resolves differently only for an inline style with a relative URL, parsed in a
    // document without a base URL, on an element since moved to a document with one. Re-parsing such styles
    // when the element moves would make this copy unnecessary.
    auto location = _location;
    location.Resolved = styleLocation.Resolved;
    return StyleCachedImage::Create(krys::move(styleLocation), CopyWithLocation(krys::move(location)));
  }

  CSSOMString ImageValue::CustomCSSText(const SerialisationContext &context) const noexcept
  {
    return _isInvalid ? CSSOMString {} : Serialise(context, _location);
  }

  bool ImageValue::Equals(const ImageValue &other) const noexcept
  {
    return _location == other._location;
  }

  bool ImageValue::CustomMayDependOnBaseUrl() const noexcept
  {
    return css::MayDependOnBaseUrl(_location);
  }

  bool ImageValue::CustomTraverseSubresources(
    KRYS_NOESCAPE const Func<bool(const CachedResource &)> &handler) const noexcept
  {
    return _cachedImage && handler(*_cachedImage);
  }
}
