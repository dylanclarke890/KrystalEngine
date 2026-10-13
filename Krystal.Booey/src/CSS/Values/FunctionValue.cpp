#include "Krystal.Booey/CSS/Values/FunctionValue.hpp"
#include "Krystal.Booey/CSS/Values/ValueId.hpp"
#include "Krystal.Core/Hash.hpp"

namespace krys::boo::css
{
  FunctionValue::FunctionValue(css::ValueId name, ValueListBuilder arguments) noexcept
      : ValueContainingList(ValueType::Function, ValueSeparator::CommaSeparator, krys::move(arguments)),
        _name(name)
  {
  }

  FunctionValue::FunctionValue(css::ValueId name) noexcept
      : ValueContainingList(ValueType::Function, ValueSeparator::CommaSeparator), _name(name)
  {
  }

  FunctionValue::FunctionValue(css::ValueId name, Ref<Value> argument) noexcept
      : ValueContainingList(ValueType::Function, ValueSeparator::CommaSeparator, krys::move(argument)),
        _name(name)
  {
  }

  FunctionValue::FunctionValue(css::ValueId name, Ref<Value> argument1, Ref<Value> argument2) noexcept
      : ValueContainingList(ValueType::Function, ValueSeparator::CommaSeparator, krys::move(argument1),
                            krys::move(argument2)),
        _name(name)
  {
  }

  FunctionValue::FunctionValue(css::ValueId name, Ref<Value> argument1, Ref<Value> argument2,
                               Ref<Value> argument3) noexcept
      : ValueContainingList(ValueType::Function, ValueSeparator::CommaSeparator, krys::move(argument1),
                            krys::move(argument2), krys::move(argument3)),
        _name(name)
  {
  }

  FunctionValue::FunctionValue(css::ValueId name, Ref<Value> argument1, Ref<Value> argument2,
                               Ref<Value> argument3, Ref<Value> argument4) noexcept
      : ValueContainingList(ValueType::Function, ValueSeparator::CommaSeparator, krys::move(argument1),
                            krys::move(argument2), krys::move(argument3), krys::move(argument4)),
        _name(name)
  {
  }

  Ref<FunctionValue> FunctionValue::Create(css::ValueId name, ValueListBuilder arguments) noexcept
  {
    return AdoptRef(*new FunctionValue(name, krys::move(arguments)));
  }

  Ref<FunctionValue> FunctionValue::Create(css::ValueId name) noexcept
  {
    return AdoptRef(*new FunctionValue(name));
  }

  Ref<FunctionValue> FunctionValue::Create(css::ValueId name, Ref<Value> argument) noexcept
  {
    return AdoptRef(*new FunctionValue(name, krys::move(argument)));
  }

  Ref<FunctionValue> FunctionValue::Create(css::ValueId name, Ref<Value> argument1,
                                           Ref<Value> argument2) noexcept
  {
    return AdoptRef(*new FunctionValue(name, krys::move(argument1), krys::move(argument2)));
  }

  Ref<FunctionValue> FunctionValue::Create(css::ValueId name, Ref<Value> argument1, Ref<Value> argument2,
                                           Ref<Value> argument3) noexcept
  {
    return AdoptRef(
      *new FunctionValue(name, krys::move(argument1), krys::move(argument2), krys::move(argument3)));
  }

  Ref<FunctionValue> FunctionValue::Create(css::ValueId name, Ref<Value> argument1, Ref<Value> argument2,
                                           Ref<Value> argument3, Ref<Value> argument4) noexcept
  {
    return AdoptRef(*new FunctionValue(name, krys::move(argument1), krys::move(argument2),
                                       krys::move(argument3), krys::move(argument4)));
  }

  CSSOMString FunctionValue::CustomCSSText(const SerialisationContext &context) const noexcept
  {
    CSSOMString result {ToString(_name)};
    result.push_back(u8'(');
    SerialiseItems(result, context);
    result.push_back(u8')');
    return result;
  }

  bool FunctionValue::AddDerivedHash(size_t &seed) const noexcept
  {
    seed = Hash::Combine(seed, _name);
    return ValueContainingList::AddDerivedHash(seed);
  }
}
