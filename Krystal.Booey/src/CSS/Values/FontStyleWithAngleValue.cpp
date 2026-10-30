#include "Krystal.Booey/CSS/Values/FontStyleWithAngleValue.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/NumericTypesSerialisation.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/NumericTypesValueVisitation.hpp"
#include "Krystal.Booey/CSS/Values/ValueId.hpp"

namespace krys::boo::css
{
  FontStyleWithAngleValue::FontStyleWithAngleValue(ObliqueAngle &&obliqueAngle) noexcept
      : Value(ValueType::FontStyleWithAngle), _obliqueAngle(krys::move(obliqueAngle))
  {
  }

  Ref<FontStyleWithAngleValue> FontStyleWithAngleValue::Create(ObliqueAngle obliqueAngle) noexcept
  {
    return AdoptRef(*new FontStyleWithAngleValue(krys::move(obliqueAngle)));
  }

  CSSOMString FontStyleWithAngleValue::CustomCSSText(const SerialisationContext &context) const noexcept
  {
    if (_obliqueAngle.IsKnownZero())
    {
      return CSSOMString(ToLower(ValueId::Normal));
    }

    CSSOMString result {ToLower(ValueId::Oblique)};
    result.push_back(u8' ');
    result.append(Serialise(context, _obliqueAngle));
    return result;
  }

  bool FontStyleWithAngleValue::Equals(const FontStyleWithAngleValue &other) const noexcept
  {
    return _obliqueAngle == other._obliqueAngle;
  }

  IterationStatus FontStyleWithAngleValue::CustomVisitChildren(
    KRYS_NOESCAPE const Func<IterationStatus(Value &)> &func) const noexcept
  {
    return VisitValueChildren(func, _obliqueAngle);
  }
}
