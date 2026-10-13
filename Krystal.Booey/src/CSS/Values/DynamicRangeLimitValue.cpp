#include "Krystal.Booey/CSS/Values/DynamicRangeLimitValue.hpp"
#include "Krystal.Booey/CSS/Values/Color/DynamicRangeLimitMix.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/NumericTypesComputedStyleDependencies.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/NumericTypesSerialisation.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/NumericTypesValueVisitation.hpp"

namespace krys::boo::css
{
  Ref<DynamicRangeLimitValue>
    DynamicRangeLimitValue::Create(css::DynamicRangeLimit dynamicRangeLimit) noexcept
  {
    return AdoptRef(*new DynamicRangeLimitValue(krys::move(dynamicRangeLimit)));
  }

  DynamicRangeLimitValue::DynamicRangeLimitValue(css::DynamicRangeLimit &&dynamicRangeLimit) noexcept
      : Value(ValueType::DynamicRangeLimit), _dynamicRangeLimit(krys::move(dynamicRangeLimit))
  {
  }

  CSSOMString DynamicRangeLimitValue::CustomCSSText(const SerialisationContext &context) const noexcept
  {
    return css::Serialise(context, _dynamicRangeLimit);
  }

  bool DynamicRangeLimitValue::Equals(const DynamicRangeLimitValue &other) const noexcept
  {
    return _dynamicRangeLimit == other._dynamicRangeLimit;
  }

  IterationStatus DynamicRangeLimitValue::CustomVisitChildren(
    KRYS_NOESCAPE const Func<IterationStatus(css::Value &)> &func) const noexcept
  {
    return css::VisitValueChildren(func, _dynamicRangeLimit);
  }
}
