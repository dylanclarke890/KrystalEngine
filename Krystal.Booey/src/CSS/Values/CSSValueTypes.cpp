#include "Krystal.Booey/CSS/Values/CSSValueTypes.hpp"
#include "Krystal.Booey/CSS/Values/CSSFunctionValue.hpp"
#include "Krystal.Booey/CSS/Values/CSSQuadValue.hpp"
#include "Krystal.Booey/CSS/Values/CSSValueList.hpp"
#include "Krystal.Booey/CSS/Values/CSSValuePair.hpp"

namespace krys::boo::css
{
  Ref<CSSValue> CreatePrimitiveCSSValue(ValueId value) noexcept
  {
    return CSSPrimitiveValue::Create(value);
  }

  Ref<CSSValue> CreatePrimitiveCSSValue(const CustomIdentifier &value) noexcept
  {
    return CSSPrimitiveValue::CreateCustomIdent(value.value);
  }

  Ref<CSSValue> CreatePrimitiveCSSValue(const PropertyIdentifier &value) noexcept
  {
    return CSSPrimitiveValue::Create(value.value);
  }

  Ref<CSSValue> CreatePrimitiveCSSValue(const CSSOMStringAtom &value) noexcept
  {
    return CSSPrimitiveValue::Create(value);
  }

  Ref<CSSValue> CreatePrimitiveCSSValue(const CSSOMString &value) noexcept
  {
    return CSSPrimitiveValue::Create(value);
  }

  Ref<CSSValue> CreateFunctionCSSValue(ValueId name, Ref<CSSValue> &&value) noexcept
  {
    return CSSFunctionValue::Create(name, krys::move(value));
  }

  template <>
  Ref<CSSValue> CreateCoalescingPairCSSValue<SerializationSeparatorType::Space>(Ref<CSSValue> &&first,
                                                                    Ref<CSSValue> &&second) noexcept
  {
    return CSSValuePair::Create(krys::move(first), krys::move(second));
  }

  template <>
  Ref<CSSValue> CreateCoalescingQuadCSSValue<SerializationSeparatorType::Space>(Ref<CSSValue> &&first,
                                                                                Ref<CSSValue> &&second,
                                                                                Ref<CSSValue> &&third, Ref<CSSValue> &&fourth) noexcept
  {
    return CSSQuadValue::Create(krys::move(first), krys::move(second), krys::move(third), krys::move(fourth));
  }

  template <>
  Ref<CSSValue> CreateListCSSValue<SerializationSeparatorType::Space>(CSSValueListBuilder &&builder) noexcept
  {
    return CSSValueList::CreateSpaceSeparated(krys::move(builder));
  }

  template <>
  Ref<CSSValue> CreateListCSSValue<SerializationSeparatorType::Comma>(CSSValueListBuilder &&builder) noexcept
  {
    return CSSValueList::CreateCommaSeparated(krys::move(builder));
  }

  template <>
  Ref<CSSValue> CreateListCSSValue<SerializationSeparatorType::Slash>(CSSValueListBuilder &&builder) noexcept
  {
    return CSSValueList::CreateSlashSeparated(krys::move(builder));
  }
}