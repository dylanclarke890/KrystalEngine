#include "Krystal.Booey/CSS/Values/ImageValue.hpp"
#include "Krystal.Booey/CSS/Values/Types.hpp"
#include "Krystal.Booey/Rendering/Style/StyleCachedImage.hpp"
#include "Krystal.Booey/Style/BuilderState.hpp"
#include "Krystal.Booey/Style/Seam.hpp"

namespace krys::boo::css
{
  ImageValue::ImageValue() noexcept : Value(ValueType::Image), _isInvalid(true)
  {
  }

  ImageValue::ImageValue(CSSURL &&location, CSSOMStringAtom &&initiatorType) noexcept
      : Value(ValueType::Image), _location(krys::move(location)), _initiatorType(krys::move(initiatorType))
  {
  }

  Ref<ImageValue> ImageValue::Create() noexcept
  {
    return AdoptRef(*new ImageValue);
  }

  Ref<ImageValue> ImageValue::Create(CSSURL location, CSSOMStringAtom initiatorType) noexcept
  {
    return AdoptRef(*new ImageValue(krys::move(location), krys::move(initiatorType)));
  }

  Ref<ImageValue> ImageValue::Create(url::URL imageURL, CSSOMStringAtom initiatorType) noexcept
  {
    CSSOMString specified {imageURL.String()};
    return Create(
      CSSURL {.Specified = krys::move(specified), .Resolved = krys::move(imageURL), .Modifiers = {}},
      krys::move(initiatorType));
  }

  Ref<ImageValue> ImageValue::CopyForComputedStyle(const CSSURL &resolvedURL) const noexcept
  {
    if (resolvedURL == _location)
    {
      return ShareRef(const_cast<ImageValue &>(*this));
    }

    auto result = Create(resolvedURL);
    // FIXME: copy the cached image once the resource loader is ported.
    result->_initiatorType = _initiatorType;
    result->_unresolvedValue = ShareRefPtr(const_cast<ImageValue *>(this));
    return result;
  }

  bool ImageValue::IsLoadedFromOpaqueSource() const noexcept
  {
    return _location.Modifiers.LoadedFromOpaqueSource == LoadedFromOpaqueSource::Yes;
  }

  bool ImageValue::CustomTraverseSubresources(
    KRYS_NOESCAPE const Func<bool(const CachedResource &)> &) const noexcept
  {
    // FIXME: return `_cachedImage && *_cachedImage && handler(**_cachedImage)` once the resource loader is
    // ported.
    return false;
  }

  bool ImageValue::CustomMayDependOnBaseUrl() const noexcept
  {
    return MayDependOnBaseURL(_location);
  }

  bool ImageValue::Equals(const ImageValue &other) const noexcept
  {
    return _location == other._location;
  }

  CSSOMString ImageValue::CustomCSSText(const SerialisationContext &context) const noexcept
  {
    if (_isInvalid)
    {
      return {};
    }
    return Serialise(context, _location);
  }

  RefPtr<StyleImage> ImageValue::CreateStyleImage(const style::BuilderState &state) const noexcept
  {
    auto styleLocation = style::ConvertToStyle(_location, state);
    if (styleLocation.Resolved == _location.Resolved)
    {
      return StyleCachedImage::Create(krys::move(styleLocation), ShareRef(const_cast<ImageValue &>(*this)));
    }

    // FIXME: This case can only happen when a element from a document with no baseURL has an inline style
    // with a relative image URL in it and has been moved to a document with a non-null baseURL. Instead of
    // re-resolving in this case, moved elements with this kind of inline style should have their inline style
    // re-parsed.
    auto newLocation = _location;
    newLocation.Resolved = styleLocation.Resolved;
    auto result = Create(krys::move(newLocation));
    // FIXME: Share the cached image, once the resource loader is ported.
    result->_initiatorType = _initiatorType;
    result->_unresolvedValue = ShareRefPtr(const_cast<ImageValue *>(this));
    return StyleCachedImage::Create(krys::move(styleLocation), krys::move(result));
  }
}
