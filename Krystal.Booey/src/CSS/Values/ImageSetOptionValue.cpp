#include "Krystal.Booey/CSS/Values/ImageSetOptionValue.hpp"

namespace krys::boo::css
{
  ImageSetOptionValue::ImageSetOptionValue(Ref<Value> &&image, Ref<PrimitiveValue> &&resolution) noexcept
      : Value(ValueType::ImageSetOption), _image(krys::move(image)), _resolution(krys::move(resolution))
  {
  }

  ImageSetOptionValue::ImageSetOptionValue(Ref<Value> &&image, Ref<PrimitiveValue> &&resolution,
                                           CSSOMString &&type) noexcept
      : Value(ValueType::ImageSetOption), _image(krys::move(image)), _resolution(krys::move(resolution)),
        _mimeType(krys::move(type))
  {
  }

  Ref<ImageSetOptionValue> ImageSetOptionValue::Create(Ref<Value> &&image) noexcept
  {
    krys_debug_assert(image->IsImageValue() || image->IsImageGeneratorValue());
    return AdoptRef(*new ImageSetOptionValue(krys::move(image), PrimitiveValue::Create(1.0, UnitType::x)));
  }

  Ref<ImageSetOptionValue> ImageSetOptionValue::Create(Ref<Value> &&image,
                                                       Ref<PrimitiveValue> &&resolution) noexcept
  {
    krys_debug_assert(image->IsImageValue() || image->IsImageGeneratorValue());
    return AdoptRef(*new ImageSetOptionValue(krys::move(image), krys::move(resolution)));
  }

  Ref<ImageSetOptionValue> ImageSetOptionValue::Create(Ref<Value> &&image, Ref<PrimitiveValue> &&resolution,
                                                       CSSOMString type) noexcept
  {
    krys_debug_assert(image->IsImageValue() || image->IsImageGeneratorValue());
    return AdoptRef(*new ImageSetOptionValue(krys::move(image), krys::move(resolution), krys::move(type)));
  }

  bool ImageSetOptionValue::Equals(const ImageSetOptionValue &other) const noexcept
  {
    if (!_image->Equals(*other._image))
    {
      return false;
    }

    if (!_resolution->Equals(*other._resolution))
    {
      return false;
    }

    if (_mimeType != other._mimeType)
    {
      return false;
    }

    return true;
  }

  CSSOMString ImageSetOptionValue::CustomCSSText(const SerialisationContext &context) const noexcept
  {
    CSSOMString result;
    result.append(_image->CssText(context));
    result.push_back(u8' ');
    result.append(_resolution->CssText(context));
    if (_mimeType)
    {
      result.append(u8" type(\"");
      result.append(*_mimeType);
      result.append(u8"\")");
    }

    return result;
  }

  void ImageSetOptionValue::SetResolution(Ref<PrimitiveValue> &&resolution) noexcept
  {
    _resolution = krys::move(resolution);
  }

  void ImageSetOptionValue::SetType(CSSOMString type) noexcept
  {
    _mimeType = krys::move(type);
  }

  bool ImageSetOptionValue::CustomTraverseSubresources(
    KRYS_NOESCAPE const Func<bool(const CachedResource &)> &handler) const noexcept
  {
    return _resolution->TraverseSubresources(handler) || _image->TraverseSubresources(handler);
  }
}
