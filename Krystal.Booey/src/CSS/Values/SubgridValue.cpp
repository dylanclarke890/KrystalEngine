#include "Krystal.Booey/CSS/Values/SubgridValue.hpp"

namespace krys::boo::css
{
  CSSOMString SubgridValue::CustomCSSText(const SerialisationContext &context) const noexcept
  {
    if (!Length())
    {
      return CSSOMString(u8"subgrid");
    }
    CSSOMString result {u8"subgrid "};
    SerialiseItems(result, context);
    return result;
  }

  SubgridValue::SubgridValue(ValueListBuilder builder) noexcept
      : ValueContainingList(ValueType::Subgrid, ValueSeparator::SpaceSeparator, krys::move(builder))
  {
  }

  Ref<SubgridValue> SubgridValue::Create(ValueListBuilder builder) noexcept
  {
    return AdoptRef(*new SubgridValue(krys::move(builder)));
  }
}
