#include "Krystal.Booey/CSS/Values/RayValue.hpp"
#include "Krystal.Booey/CSS/PrimitiveValueMappings.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/NumericTypesSerialisation.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/NumericTypesValueVisitation.hpp"
#include "Krystal.Booey/CSS/Values/ValueId.hpp"

namespace krys::boo::css
{
  RayValue::RayValue(RayFunction &&ray, CSSBoxType coordinateBox) noexcept
      : Value(ValueType::Ray), _ray(krys::move(ray)), _coordinateBox(coordinateBox)
  {
  }

  Ref<RayValue> RayValue::Create(RayFunction ray, CSSBoxType coordinateBox) noexcept
  {
    return AdoptRef(*new RayValue(krys::move(ray), coordinateBox));
  }

  CSSOMString RayValue::CustomCSSText(const SerialisationContext &context) const noexcept
  {
    CSSOMString result = Serialise(context, _ray);

    if (_coordinateBox != CSSBoxType::BoxMissing)
    {
      result.push_back(u8' ');
      result.append(ToLower(ToValueId(_coordinateBox)));
    }

    return result;
  }

  bool RayValue::Equals(const RayValue &other) const noexcept
  {
    return _ray == other._ray && _coordinateBox == other._coordinateBox;
  }

  IterationStatus
    RayValue::CustomVisitChildren(KRYS_NOESCAPE const Func<IterationStatus(Value &)> &func) const noexcept
  {
    return VisitValueChildren(func, _ray);
  }
}
