#include "Krystal.HTML/CSS/Values/CSSValueTypes.hpp"
#include "Krystal.HTML/CSS/Values/CSSValueList.hpp"

namespace Krys::HTML
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
    return CSSFunctionValue::Create(name, Krys::Move(value));
  }

  template <>
  Ref<CSSValue> CreateCoalescingPairCSSValue<SerializationSeparatorType::Space>(Ref<CSSValue> &&first,
                                                                                Ref<CSSValue> &&second)
  {
    return CSSValuePair::Create(Krys::Move(first), Krys::Move(second));
  }

  template <>
  Ref<CSSValue> CreateCoalescingQuadCSSValue<SerializationSeparatorType::Space>(Ref<CSSValue> &&first,
                                                                                Ref<CSSValue> &&second,
                                                                                Ref<CSSValue> &&third,
                                                                                Ref<CSSValue> &&fourth)
  {
    return CSSQuadValue::Create(Krys::Move(first), Krys::Move(second), Krys::Move(third), Krys::Move(fourth));
  }

  template <>
  Ref<CSSValue> CreateListCSSValue<SerializationSeparatorType::Space>(CSSValueListBuilder &&builder)
  {
    return CSSValueList::CreateSpaceSeparated(Krys::Move(builder));
  }

  template <>
  Ref<CSSValue> CreateListCSSValue<SerializationSeparatorType::Comma>(CSSValueListBuilder &&builder)
  {
    return CSSValueList::CreateCommaSeparated(Krys::Move(builder));
  }

  template <>
  Ref<CSSValue> CreateListCSSValue<SerializationSeparatorType::Slash>(CSSValueListBuilder &&builder)
  {
    return CSSValueList::CreateSlashSeparated(Krys::Move(builder));
  }
}