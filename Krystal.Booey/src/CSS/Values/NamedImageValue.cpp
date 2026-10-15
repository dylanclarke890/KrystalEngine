#include "Krystal.Booey/CSS/Values/NamedImageValue.hpp"
#include "Krystal.Booey/Rendering/Style/StyleNamedImage.hpp"
#include "Krystal.Booey/Style/BuilderState.hpp"

namespace krys::boo::css
{
  NamedImageValue::~NamedImageValue() noexcept = default;

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

  RefPtr<StyleImage> NamedImageValue::CreateStyleImage(const style::BuilderState &) const noexcept
  {
    if (_cachedStyleImage)
    {
      return _cachedStyleImage;
    }

    _cachedStyleImage = StyleNamedImage::Create(_name);
    return _cachedStyleImage;
  }
}
