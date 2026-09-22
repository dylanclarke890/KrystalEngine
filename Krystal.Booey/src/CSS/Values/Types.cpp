#include "Krystal.Booey/CSS/Values/Types.hpp"
#include "Krystal.Booey/CSS/Values/FunctionValue.hpp"
#include "Krystal.Booey/CSS/Values/PrimitiveValue.hpp"
#include "Krystal.Booey/CSS/Values/QuadValue.hpp"
#include "Krystal.Booey/CSS/Values/ValueList.hpp"
#include "Krystal.Booey/CSS/Values/ValuePair.hpp"

namespace krys::boo::css
{
#pragma region Serialize

  void SerializeCustomIdentifier(CSSOMString &builder, const SerializationContext &context,
                                 const CustomIdentifier &identifier) noexcept
  {
    // TODO: Implement serialization for CustomIdentifier
    (void)builder;
    (void)context;
    (void)identifier;
    krys_not_implemented();
  }

  void SerializePropertyIdentifier(CSSOMString &builder, const SerializationContext &context,
                                   const PropertyIdentifier &identifier) noexcept
  {
    // TODO: Implement serialization for PropertyIdentifier
    (void)builder;
    (void)context;
    (void)identifier;
    krys_not_implemented();
  }

  void SerializeString(CSSOMString &builder, const SerializationContext &context,
                       const CSSOMStringAtom &atom) noexcept
  {
    // TODO: Implement serialization for CSSOMStringAtom
    (void)builder;
    (void)context;
    (void)atom;
    krys_not_implemented();
  }

  void SerializeString(CSSOMString &builder, const SerializationContext &context,
                       const CSSOMString &string) noexcept
  {
    // TODO: Implement serialization for CSSOMString
    (void)builder;
    (void)context;
    (void)string;
    krys_not_implemented();
  }

#pragma endregion

#pragma region ValueCreation

  Ref<Value> CreatePrimitiveValue(ValueId value) noexcept
  {
    return PrimitiveValue::Create(value);
  }

  Ref<Value> CreatePrimitiveValue(const CustomIdentifier &value) noexcept
  {
    return PrimitiveValue::CreateCustomIdent(value.value);
  }

  Ref<Value> CreatePrimitiveValue(const PropertyIdentifier &value) noexcept
  {
    return PrimitiveValue::Create(value.value);
  }

  Ref<Value> CreatePrimitiveValue(const CSSOMStringAtom &value) noexcept
  {
    return PrimitiveValue::Create(value);
  }

  Ref<Value> CreatePrimitiveValue(const CSSOMString &value) noexcept
  {
    return PrimitiveValue::Create(value);
  }

  Ref<Value> CreateFunctionValue(ValueId name, Ref<Value> &&value) noexcept
  {
    return FunctionValue::Create(name, krys::move(value));
  }

  template <>
  Ref<Value> CreateCoalescingPairValue<SerializationSeparatorType::Space>(Ref<Value> &&first,
                                                                          Ref<Value> &&second) noexcept
  {
    return ValuePair::Create(krys::move(first), krys::move(second));
  }

  template <>
  Ref<Value> CreateCoalescingQuadValue<SerializationSeparatorType::Space>(Ref<Value> &&first,
                                                                          Ref<Value> &&second,
                                                                          Ref<Value> &&third,
                                                                          Ref<Value> &&fourth) noexcept
  {
    return QuadValue::Create(krys::move(first), krys::move(second), krys::move(third), krys::move(fourth));
  }

  template <>
  Ref<Value> CreateListValue<SerializationSeparatorType::Space>(ValueListBuilder &&builder) noexcept
  {
    return ValueList::CreateSpaceSeparated(krys::move(builder));
  }

  template <>
  Ref<Value> CreateListValue<SerializationSeparatorType::Comma>(ValueListBuilder &&builder) noexcept
  {
    return ValueList::CreateCommaSeparated(krys::move(builder));
  }

  template <>
  Ref<Value> CreateListValue<SerializationSeparatorType::Slash>(ValueListBuilder &&builder) noexcept
  {
    return ValueList::CreateSlashSeparated(krys::move(builder));
  }

#pragma endregion
}