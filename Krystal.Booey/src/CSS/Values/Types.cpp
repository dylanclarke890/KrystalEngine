#include "Krystal.Booey/CSS/Values/Types.hpp"
#include "Krystal.Booey/CSS/Values/FunctionValue.hpp"
#include "Krystal.Booey/CSS/Values/QuadValue.hpp"
#include "Krystal.Booey/CSS/Values/ValueList.hpp"
#include "Krystal.Booey/CSS/Values/ValuePair.hpp"

namespace krys::boo::css
{
  Ref<Value> CreatePrimitiveCSSValue(ValueId value) noexcept
  {
    return PrimitiveValue::Create(value);
  }

  Ref<Value> CreatePrimitiveCSSValue(const CustomIdentifier &value) noexcept
  {
    return PrimitiveValue::CreateCustomIdent(value.value);
  }

  Ref<Value> CreatePrimitiveCSSValue(const PropertyIdentifier &value) noexcept
  {
    return PrimitiveValue::Create(value.value);
  }

  Ref<Value> CreatePrimitiveCSSValue(const CSSOMStringAtom &value) noexcept
  {
    return PrimitiveValue::Create(value);
  }

  Ref<Value> CreatePrimitiveCSSValue(const CSSOMString &value) noexcept
  {
    return PrimitiveValue::Create(value);
  }

  Ref<Value> CreateFunctionCSSValue(ValueId name, Ref<Value> &&value) noexcept
  {
    return FunctionValue::Create(name, krys::move(value));
  }

  template <>
  Ref<Value>
    CreateCoalescingPairCSSValue<SerializationSeparatorType::Space>(Ref<Value> &&first,
                                                                    Ref<Value> &&second) noexcept
  {
    return ValuePair::Create(krys::move(first), krys::move(second));
  }

  template <>
  Ref<Value> CreateCoalescingQuadCSSValue<SerializationSeparatorType::Space>(
    Ref<Value> &&first, Ref<Value> &&second, Ref<Value> &&third, Ref<Value> &&fourth) noexcept
  {
    return QuadValue::Create(krys::move(first), krys::move(second), krys::move(third), krys::move(fourth));
  }

  template <>
  Ref<Value> CreateListCSSValue<SerializationSeparatorType::Space>(ValueListBuilder &&builder) noexcept
  {
    return ValueList::CreateSpaceSeparated(krys::move(builder));
  }

  template <>
  Ref<Value> CreateListCSSValue<SerializationSeparatorType::Comma>(ValueListBuilder &&builder) noexcept
  {
    return ValueList::CreateCommaSeparated(krys::move(builder));
  }

  template <>
  Ref<Value> CreateListCSSValue<SerializationSeparatorType::Slash>(ValueListBuilder &&builder) noexcept
  {
    return ValueList::CreateSlashSeparated(krys::move(builder));
  }
}