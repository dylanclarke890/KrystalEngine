#include "Krystal.Booey/CSS/Values/CanvasValue.hpp"
#include "Krystal.Booey/Rendering/Style/StyleCanvasImage.hpp"

namespace krys::boo::css
{
  CanvasValue::~CanvasValue() noexcept = default;

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

  RefPtr<StyleImage> CanvasValue::CreateStyleImage(const style::BuilderState &) const noexcept
  {
    if (_cachedStyleImage)
    {
      return _cachedStyleImage;
    }

    _cachedStyleImage = StyleCanvasImage::Create(_name);
    return _cachedStyleImage;
  }
}
