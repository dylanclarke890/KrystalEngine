#include "Krystal.Booey/CSS/Values/FunctionValue.hpp"
#include "Krystal.Booey/CSS/Values/PrimitiveValue.hpp"
#include "Krystal.Booey/CSS/Values/QuadValue.hpp"
#include "Krystal.Booey/CSS/Values/Types.hpp"
#include "Krystal.Booey/CSS/Values/ValueList.hpp"
#include "Krystal.Booey/CSS/Values/ValuePair.hpp"
#include "Krystal.Booey/CSS/Values/ValuePool.hpp"

namespace krys::boo::css
{
  Ref<Value> CreatePrimitiveValue(ValueId value) noexcept
  {
    return PrimitiveValue::Create(value);
  }

  Ref<Value> CreatePrimitiveValue(const CustomIdentifier &value) noexcept
  {
    return PrimitiveValue::CreateCustomIdent(value.Value);
  }

  Ref<Value> CreatePrimitiveValue(const PropertyIdentifier &value) noexcept
  {
    return PrimitiveValue::Create(value.Value);
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
  Ref<Value> CreateCoalescingPairValue<SerialisationSeparatorType::Space>(Ref<Value> &&first,
                                                                          Ref<Value> &&second) noexcept
  {
    return ValuePair::Create(krys::move(first), krys::move(second));
  }

  template <>
  Ref<Value> CreateCoalescingQuadValue<SerialisationSeparatorType::Space>(Ref<Value> &&first,
                                                                          Ref<Value> &&second,
                                                                          Ref<Value> &&third,
                                                                          Ref<Value> &&fourth) noexcept
  {
    return QuadValue::Create(krys::move(first), krys::move(second), krys::move(third), krys::move(fourth));
  }

  template <>
  Ref<Value> CreateListValue<SerialisationSeparatorType::Space>(ValueListBuilder &&builder) noexcept
  {
    return ValueList::CreateSpaceSeparated(krys::move(builder));
  }

  template <>
  Ref<Value> CreateListValue<SerialisationSeparatorType::Comma>(ValueListBuilder &&builder) noexcept
  {
    return ValueList::CreateCommaSeparated(krys::move(builder));
  }

  template <>
  Ref<Value> CreateListValue<SerialisationSeparatorType::Slash>(ValueListBuilder &&builder) noexcept
  {
    return ValueList::CreateSlashSeparated(krys::move(builder));
  }
}