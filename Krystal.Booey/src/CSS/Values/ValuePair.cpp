#include "Krystal.Booey/CSS/Values/ValuePair.hpp"
#include "Krystal.Core/Hash.hpp"

namespace krys::boo::css
{
  ValuePair::ValuePair(ValueSeparator separator, Ref<Value> first, Ref<Value> second,
                       IdenticalValueSerialisation serialisation) noexcept
      : Value(ValueType::ValuePair),
        _coalesceIdenticalValues(serialisation != IdenticalValueSerialisation::DoNotCoalesce),
        _first(krys::move(first)), _second(krys::move(second))
  {
    _valueSeparator = separator;
  }

  Ref<ValuePair> ValuePair::Create(Ref<Value> first, Ref<Value> second) noexcept
  {
    return AdoptRef(*new ValuePair(ValueSeparator::SpaceSeparator, krys::move(first), krys::move(second),
                                   IdenticalValueSerialisation::Coalesce));
  }

  Ref<ValuePair> ValuePair::CreateSlashSeparated(Ref<Value> first, Ref<Value> second) noexcept
  {
    return AdoptRef(*new ValuePair(ValueSeparator::SlashSeparator, krys::move(first), krys::move(second),
                                   IdenticalValueSerialisation::DoNotCoalesce));
  }

  Ref<ValuePair> ValuePair::CreateNonCoalescing(Ref<Value> first, Ref<Value> second) noexcept
  {
    return AdoptRef(*new ValuePair(ValueSeparator::SpaceSeparator, krys::move(first), krys::move(second),
                                   IdenticalValueSerialisation::DoNotCoalesce));
  }

  bool ValuePair::CanBeCoalesced() const noexcept
  {
    return _coalesceIdenticalValues && _first->Equals(*_second);
  }

  CSSOMString ValuePair::CustomCSSText(const SerialisationContext &context) const noexcept
  {
    auto first = _first->CssText(context);
    auto second = _second->CssText(context);
    if (_coalesceIdenticalValues && first == second)
    {
      return first;
    }
    return first.append(SeparatorCSSText()).append(second);
  }

  bool ValuePair::Equals(const ValuePair &other) const noexcept
  {
    return _valueSeparator == other._valueSeparator
           && _coalesceIdenticalValues == other._coalesceIdenticalValues && _first->Equals(*other._first)
           && _second->Equals(*other._second);
  }

  bool ValuePair::AddDerivedHash(size_t &seed) const noexcept
  {
    seed = Hash::Combine(seed, _valueSeparator, _coalesceIdenticalValues);
    return _first->AddHash(seed) && _second->AddHash(seed);
  }
}
