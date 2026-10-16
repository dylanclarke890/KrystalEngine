#include "Krystal.Booey/CSS/Values/SubgridValue.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"

namespace krys::boo::css
{
  CSSOMString SubgridValue::CustomCSSText(const SerialisationContext &context) const noexcept
  {
    if (!Length())
    {
      return CSSOMString(u8"subgrid");
    }
    StringBuilder result;
    result.Append(u8"subgrid ");
    SerialiseItems(result, context);
    return krys::move(result).ToString();
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
