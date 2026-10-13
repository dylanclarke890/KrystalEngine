#include "Krystal.Booey/CSS/Values/CursorImageValue.hpp"
#include "Krystal.Booey/CSS/Values/ImageValue.hpp"

namespace krys::boo::css
{
  Ref<CursorImageValue> CursorImageValue::Create(Ref<Value> &&value, RefPtr<Value> &&hotSpot) noexcept
  {
    auto *imageValue = DynamicDowncast<ImageValue>(value.get());
    auto originalURL = imageValue ? imageValue->URL() : CSSURL::None();
    return AdoptRef(*new CursorImageValue(krys::move(value), krys::move(hotSpot), krys::move(originalURL)));
  }

  Ref<CursorImageValue> CursorImageValue::Create(Ref<Value> &&imageValue, RefPtr<Value> &&hotSpot,
                                                 CSSURL &&originalURL) noexcept
  {
    return AdoptRef(
      *new CursorImageValue(krys::move(imageValue), krys::move(hotSpot), krys::move(originalURL)));
  }

  CursorImageValue::CursorImageValue(Ref<Value> &&imageValue, RefPtr<Value> &&hotSpot,
                                     CSSURL &&originalURL) noexcept
      : Value(ValueType::CursorImage), _originalURL(krys::move(originalURL)),
        _imageValue(krys::move(imageValue)), _hotSpot(krys::move(hotSpot))
  {
  }

  CSSOMString CursorImageValue::CustomCSSText(const SerialisationContext &context) const noexcept
  {
    auto text = _imageValue->CssText(context);
    if (!_hotSpot)
    {
      return text;
    }
    text.push_back(u8' ');
    text.append(_hotSpot->First().CssText(context));
    text.push_back(u8' ');
    text.append(_hotSpot->Second().CssText(context));
    return text;
  }

  bool CursorImageValue::Equals(const CursorImageValue &other) const noexcept
  {
    return CompareValue(_imageValue, other._imageValue) && CompareValuePtr(_hotSpot, other._hotSpot);
  }
}
