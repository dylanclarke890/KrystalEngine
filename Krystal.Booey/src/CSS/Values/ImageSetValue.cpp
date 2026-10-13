#include "Krystal.Booey/CSS/Values/ImageSetValue.hpp"
#include "Krystal.Booey/CSS/Values/ImageSetOptionValue.hpp"

namespace krys::boo::css
{
  Ref<ImageSetValue> ImageSetValue::Create(ValueListBuilder builder) noexcept
  {
    return AdoptRef(*new ImageSetValue(krys::move(builder)));
  }

  ImageSetValue::ImageSetValue(ValueListBuilder builder) noexcept
      : ValueContainingList(ValueType::ImageSet, ValueSeparator::CommaSeparator, krys::move(builder))
  {
  }

  CSSOMString ImageSetValue::CustomCSSText(const SerialisationContext &context) const noexcept
  {
    CSSOMString result {u8"image-set("};
    for (uint32 i = 0u; i < Length(); ++i)
    {
      if (i > 0u)
      {
        result.append(u8", ");
      }
      krys_debug_assert(Is<ImageSetOptionValue>(Item(i)));
      result.append(Item(i)->CssText(context));
    }
    result.push_back(u8')');
    return result;
  }
}
