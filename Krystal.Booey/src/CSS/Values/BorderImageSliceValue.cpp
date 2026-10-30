#include "Krystal.Booey/CSS/Values/BorderImageSliceValue.hpp"

namespace krys::boo::css
{
  BorderImageSliceValue::BorderImageSliceValue(Quad slices, bool fill) noexcept
      : Value(ValueType::BorderImageSlice), _slices(krys::move(slices)), _fill(fill)
  {
  }

  Ref<BorderImageSliceValue> BorderImageSliceValue::Create(Quad slices, bool fill) noexcept
  {
    return AdoptRef(*new BorderImageSliceValue(krys::move(slices), fill));
  }

  CSSOMString BorderImageSliceValue::CustomCSSText(const SerialisationContext &context) const noexcept
  {
    if (_fill)
    {
      return _slices.CssText(context) + u8" fill";
    }
    return _slices.CssText(context);
  }

  bool BorderImageSliceValue::Equals(const BorderImageSliceValue &other) const noexcept
  {
    return _fill == other._fill && _slices.Equals(other._slices);
  }
}
