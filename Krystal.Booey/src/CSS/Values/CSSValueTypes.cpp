#include "Krystal.Booey/CSS/Values/CSSValueTypes.hpp"
#include "Krystal.Booey/CSS/Values/CSSValueList.hpp"

namespace krys::boo::css
{
  Ref<CSSValue> CreatePrimitiveCSSValue(CSSValueId value)
  {
    return CSSPrimitiveValue::Create(value);
  }

  Ref<CSSValue> CreatePrimitiveCSSValue(const CustomIdentifier &value)
  {
    return CSSPrimitiveValue::CreateCustomIdent(value.value);
  }

  Ref<CSSValue> CreatePrimitiveCSSValue(const PropertyIdentifier &value)
  {
    return CSSPrimitiveValue::Create(value.value);
  }

  Ref<CSSValue> CreatePrimitiveCSSValue(const CSSOMStringAtom &value)
  {
    return CSSPrimitiveValue::Create(value);
  }

  Ref<CSSValue> CreatePrimitiveCSSValue(const CSSOMString &value)
  {
    return CSSPrimitiveValue::Create(value);
  }

  Ref<CSSValue> CreateFunctionCSSValue(CSSValueId name, Ref<CSSValue> &&value)
  {
    return CSSFunctionValue::Create(name, krys::move(value));
  }

  template <>
  Ref<CSSValue> CreateCoalescingPairCSSValue<SerializationSeparatorType::Space>(Ref<CSSValue> &&first,
                                                                                Ref<CSSValue> &&second)
  {
    return CSSValuePair::Create(krys::move(first), krys::move(second));
  }

  template <>
  Ref<CSSValue> CreateCoalescingQuadCSSValue<SerializationSeparatorType::Space>(Ref<CSSValue> &&first,
                                                                                Ref<CSSValue> &&second,
                                                                                Ref<CSSValue> &&third,
                                                                                Ref<CSSValue> &&fourth)
  {
    return CSSQuadValue::Create(krys::move(first), krys::move(second), krys::move(third), krys::move(fourth));
  }

  template <>
  Ref<CSSValue> CreateListCSSValue<SerializationSeparatorType::Space>(CSSValueListBuilder &&builder)
  {
    return CSSValueList::CreateSpaceSeparated(krys::move(builder));
  }

  template <>
  Ref<CSSValue> CreateListCSSValue<SerializationSeparatorType::Comma>(CSSValueListBuilder &&builder)
  {
    return CSSValueList::CreateCommaSeparated(krys::move(builder));
  }

  template <>
  Ref<CSSValue> CreateListCSSValue<SerializationSeparatorType::Slash>(CSSValueListBuilder &&builder)
  {
    return CSSValueList::CreateSlashSeparated(krys::move(builder));
  }
}