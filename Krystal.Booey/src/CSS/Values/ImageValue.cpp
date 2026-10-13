#include "Krystal.Booey/CSS/Values/ImageValue.hpp"
#include "Krystal.Booey/CSS/Values/Types.hpp"

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
}
