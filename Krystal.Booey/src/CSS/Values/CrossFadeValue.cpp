#include "Krystal.Booey/CSS/Values/CrossFadeValue.hpp"
#include "Krystal.Booey/Rendering/Style/StyleCrossfadeImage.hpp"
#include "Krystal.Booey/Style/BuilderState.hpp"

namespace krys::boo::css
{
  CrossFadeValue::CrossFadeValue(Ref<Value> &&fromValueOrNone, Ref<Value> &&toValueOrNone,
                                 Ref<PrimitiveValue> &&percentageValue, bool isPrefixed) noexcept
      : Value(ValueType::CrossFade), _fromValueOrNone(krys::move(fromValueOrNone)),
        _toValueOrNone(krys::move(toValueOrNone)), _percentageValue(krys::move(percentageValue)),
        _isPrefixed(isPrefixed)
  {
  }

  Ref<CrossFadeValue> CrossFadeValue::Create(Ref<Value> &&fromValueOrNone, Ref<Value> &&toValueOrNone,
                                             Ref<PrimitiveValue> &&percentageValue, bool isPrefixed) noexcept
  {
    return AdoptRef(*new CrossFadeValue(krys::move(fromValueOrNone), krys::move(toValueOrNone),
                                        krys::move(percentageValue), isPrefixed));
  }

  bool CrossFadeValue::Equals(const CrossFadeValue &other) const noexcept
  {
    return EqualInputImages(other) && CompareValue(_percentageValue, other._percentageValue);
  }

  bool CrossFadeValue::EqualInputImages(const CrossFadeValue &other) const noexcept
  {
    return CompareValue(_fromValueOrNone, other._fromValueOrNone)
           && CompareValue(_toValueOrNone, other._toValueOrNone);
  }

  CSSOMString CrossFadeValue::CustomCSSText(const SerialisationContext &context) const noexcept
  {
    CSSOMString result {_isPrefixed ? u8"-webkit-cross-fade(" : u8"cross-fade("};
    result.append(_fromValueOrNone->CssText(context));
    result.append(u8", ");
    result.append(_toValueOrNone->CssText(context));
    result.append(u8", ");
    result.append(_percentageValue->CssText(context));
    result.push_back(u8')');
    return result;
  }

  RefPtr<StyleImage> CrossFadeValue::CreateStyleImage(const style::BuilderState &state) const noexcept
  {
    return StyleCrossfadeImage::Create(
      state.CreateStyleImage(*_fromValueOrNone), state.CreateStyleImage(*_toValueOrNone),
      std::clamp(_percentageValue->ValueDividingBy100IfPercentage<double>(state.CssToLengthConversionData()),
                 0.0, 1.0),
      _isPrefixed);
  }
}
