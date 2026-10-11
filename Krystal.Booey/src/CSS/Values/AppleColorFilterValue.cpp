#include "Krystal.Booey/CSS/Values/AppleColorFilterValue.hpp"
#include "Krystal.Booey/CSS/Values/Types.hpp"

namespace krys::boo::css
{
  AppleColorFilterValue::AppleColorFilterValue(AppleColorFilter filter) noexcept
      : css::Value(ValueType::AppleColorFilter), _filter(std::move(filter))
  {
  }

  Ref<AppleColorFilterValue> AppleColorFilterValue::Create(AppleColorFilter filter) noexcept
  {
    return AdoptRef(*new AppleColorFilterValue(krys::move(filter)));
  }

  CSSOMString AppleColorFilterValue::CustomCSSText(const SerialisationContext &context) const noexcept
  {
    return css::Serialise(context, _filter);
  }

  bool AppleColorFilterValue::Equals(const AppleColorFilterValue &other) const noexcept
  {
    return _filter == other._filter;
  }

  IterationStatus
    AppleColorFilterValue::CustomVisitChildren(const Func<IterationStatus(css::Value &)> &func) const noexcept
  {
    return css::VisitValueChildren(func, _filter);
  }
}