#include "Krystal.Booey/CSS/Values/GridAutoRepeatValue.hpp"
#include "Krystal.Booey/CSS/Values/ValueId.hpp"
#include "Krystal.Core/Assert.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"

namespace krys::boo::css
{
  GridAutoRepeatValue::GridAutoRepeatValue(bool isAutoFit, ValueListBuilder builder) noexcept
      : ValueContainingList(ValueType::GridAutoRepeat, ValueSeparator::SpaceSeparator, krys::move(builder)),
        _isAutoFit(isAutoFit)
  {
  }

  Ref<GridAutoRepeatValue> GridAutoRepeatValue::Create(css::ValueId id, ValueListBuilder builder) noexcept
  {
    krys_debug_assert(id == ValueId::AutoFill || id == ValueId::AutoFit);
    return AdoptRef(*new GridAutoRepeatValue(id == ValueId::AutoFit, krys::move(builder)));
  }

  css::ValueId GridAutoRepeatValue::AutoRepeatID() const noexcept
  {
    return _isAutoFit ? ValueId::AutoFit : ValueId::AutoFill;
  }

  CSSOMString GridAutoRepeatValue::CustomCSSText(const SerialisationContext &context) const noexcept
  {
    StringBuilder result;
    result.Append(u8"repeat(");
    result.Append(ToString(AutoRepeatID()));
    result.Append(u8", ");
    SerialiseItems(result, context);
    result.Append(u8')');
    return krys::move(result).ToString();
  }

  bool GridAutoRepeatValue::Equals(const GridAutoRepeatValue &other) const noexcept
  {
    return _isAutoFit == other._isAutoFit && ItemsEqual(other);
  }
}
