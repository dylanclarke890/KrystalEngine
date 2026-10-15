#include "Krystal.Booey/CSS/Values/PaintImageValue.hpp"
#include "Krystal.Booey/Rendering/Style/StylePaintImage.hpp"

namespace krys::boo::css
{
  PaintImageValue::PaintImageValue(CSSOMString &&name, Ref<VariableData> &&arguments) noexcept
      : Value(ValueType::PaintImage), _name(krys::move(name)), _arguments(krys::move(arguments))
  {
  }

  Ref<PaintImageValue> PaintImageValue::Create(CSSOMString name, Ref<VariableData> arguments) noexcept
  {
    return AdoptRef(*new PaintImageValue(krys::move(name), krys::move(arguments)));
  }

  CSSOMString PaintImageValue::CustomCSSText(const SerialisationContext &) const noexcept
  {
    // FIXME: This should include the arguments too.
    CSSOMString result {u8"paint("};
    result.append(_name);
    result.push_back(u8')');
    return result;
  }

  RefPtr<StyleImage> PaintImageValue::CreateStyleImage(const style::BuilderState &) const noexcept
  {
    return StylePaintImage::Create(_name, _arguments);
  }
}
