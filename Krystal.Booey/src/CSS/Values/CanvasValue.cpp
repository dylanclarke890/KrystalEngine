#include "Krystal.Booey/CSS/Values/CanvasValue.hpp"

namespace krys::boo::css
{
  CanvasValue::CanvasValue(CSSOMString &&name) noexcept : Value(ValueType::Canvas), _name(krys::move(name))
  {
  }

  Ref<CanvasValue> CanvasValue::Create(CSSOMString name) noexcept
  {
    return AdoptRef(*new CanvasValue(krys::move(name)));
  }

  CSSOMString CanvasValue::CustomCSSText(const SerialisationContext &) const noexcept
  {
    CSSOMString result {u8"-webkit-canvas("};
    result.append(_name);
    result.push_back(u8')');
    return result;
  }

  bool CanvasValue::Equals(const CanvasValue &other) const noexcept
  {
    return _name == other._name;
  }
}
