#include "Krystal.Booey/CSS/Values/FilterValue.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/NumericTypesComputedStyleDependencies.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/NumericTypesSerialisation.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/NumericTypesValueVisitation.hpp"

namespace krys::boo::css
{
  Ref<FilterValue> FilterValue::Create(boo::css::Filter filter) noexcept
  {
    return AdoptRef(*new FilterValue(krys::move(filter)));
  }

  FilterValue::FilterValue(boo::css::Filter &&filter) noexcept
      : Value(ValueType::Filter), _filter(krys::move(filter))
  {
  }

  CSSOMString FilterValue::CustomCSSText(const SerialisationContext &context) const noexcept
  {
    return Serialise(context, _filter);
  }

  bool FilterValue::Equals(const FilterValue &other) const noexcept
  {
    return _filter == other._filter;
  }

  IterationStatus
    FilterValue::CustomVisitChildren(KRYS_NOESCAPE const Func<IterationStatus(Value &)> &func) const noexcept
  {
    return VisitValueChildren(func, _filter);
  }
}
