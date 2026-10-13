#include "Krystal.Booey/CSS/Values/NamedImageValue.hpp"

namespace krys::boo::css
{
  NamedImageValue::NamedImageValue(CSSOMString &&name) noexcept
      : Value(ValueType::NamedImage), _name(krys::move(name))
  {
  }

  Ref<NamedImageValue> NamedImageValue::Create(CSSOMString name) noexcept
  {
    return AdoptRef(*new NamedImageValue(krys::move(name)));
  }

  CSSOMString NamedImageValue::CustomCSSText(const SerialisationContext &) const noexcept
  {
    CSSOMString result {u8"-webkit-named-image("};
    result.append(_name);
    result.push_back(u8')');
    return result;
  }

  bool NamedImageValue::Equals(const NamedImageValue &other) const noexcept
  {
    return _name == other._name;
  }
}
