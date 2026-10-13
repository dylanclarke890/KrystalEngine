#include "Krystal.Booey/CSS/Values/AppleColorFilterValue.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/NumericTypesComputedStyleDependencies.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/NumericTypesSerialisation.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/NumericTypesValueVisitation.hpp"

namespace krys::boo::css
{
  Ref<AppleColorFilterValue> AppleColorFilterValue::Create(AppleColorFilter filter) noexcept
  {
    return AdoptRef(*new AppleColorFilterValue(krys::move(filter)));
  }

  AppleColorFilterValue::AppleColorFilterValue(AppleColorFilter &&filter) noexcept
      : Value(ValueType::AppleColorFilter), _filter(krys::move(filter))
  {
  }

  CSSOMString AppleColorFilterValue::CustomCSSText(const SerialisationContext &context) const noexcept
  {
    return Serialise(context, _filter);
  }

  bool AppleColorFilterValue::Equals(const AppleColorFilterValue &other) const noexcept
  {
    return _filter == other._filter;
  }

  IterationStatus AppleColorFilterValue::CustomVisitChildren(
    KRYS_NOESCAPE const Func<IterationStatus(Value &)> &func) const noexcept
  {
    return VisitValueChildren(func, _filter);
  }
}
