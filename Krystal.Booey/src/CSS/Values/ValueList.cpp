#include "Krystal.Booey/CSS/Values/ValueList.hpp"
#include "Krystal.Booey/CSS/Values/PrimitiveValue.hpp"
#include "Krystal.Core/Hash.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"
#include <limits>

namespace krys::boo::css
{
  ValueContainingList::ValueContainingList(ValueType type, ValueSeparator separator) noexcept : Value(type)
  {
    _valueSeparator = separator;
  }

  ValueContainingList::ValueContainingList(ValueType type, ValueSeparator separator,
                                           ValueListBuilder values) noexcept
      : Value(type)
  {
    _valueSeparator = separator;

    krys_assert(values.size() <= std::numeric_limits<uint32>::max());
    _storage.reserve(values.size());
    for (auto &value : values)
    {
      _storage.push_back(value.release());
    }
  }

  ValueContainingList::ValueContainingList(ValueType type, ValueSeparator separator,
                                           Ref<Value> value) noexcept
      : Value(type)
  {
    _valueSeparator = separator;
    _storage.push_back(value.release());
  }

  ValueContainingList::ValueContainingList(ValueType type, ValueSeparator separator, Ref<Value> value1,
                                           Ref<Value> value2) noexcept
      : Value(type)
  {
    _valueSeparator = separator;
    _storage.push_back(value1.release());
    _storage.push_back(value2.release());
  }

  ValueContainingList::ValueContainingList(ValueType type, ValueSeparator separator, Ref<Value> value1,
                                           Ref<Value> value2, Ref<Value> value3) noexcept
      : Value(type)
  {
    _valueSeparator = separator;
    _storage.push_back(value1.release());
    _storage.push_back(value2.release());
    _storage.push_back(value3.release());
  }

  ValueContainingList::ValueContainingList(ValueType type, ValueSeparator separator, Ref<Value> value1,
                                           Ref<Value> value2, Ref<Value> value3, Ref<Value> value4) noexcept
      : Value(type)
  {
    _valueSeparator = separator;
    _storage.push_back(value1.release());
    _storage.push_back(value2.release());
    _storage.push_back(value3.release());
    _storage.push_back(value4.release());
  }

  ValueContainingList::~ValueContainingList() noexcept
  {
    for (auto &value : *this)
    {
      value.SubRef();
    }
  }

  ValueList::ValueList(ValueSeparator separator) noexcept
      : ValueContainingList(ValueType::ValueList, separator)
  {
  }

  ValueList::ValueList(ValueSeparator separator, ValueListBuilder values) noexcept
      : ValueContainingList(ValueType::ValueList, separator, krys::move(values))
  {
  }

  ValueList::ValueList(ValueSeparator separator, Ref<Value> value) noexcept
      : ValueContainingList(ValueType::ValueList, separator, krys::move(value))
  {
  }

  ValueList::ValueList(ValueSeparator separator, Ref<Value> value1, Ref<Value> value2) noexcept
      : ValueContainingList(ValueType::ValueList, separator, krys::move(value1), krys::move(value2))
  {
  }

  ValueList::ValueList(ValueSeparator separator, Ref<Value> value1, Ref<Value> value2,
                       Ref<Value> value3) noexcept
      : ValueContainingList(ValueType::ValueList, separator, krys::move(value1), krys::move(value2),
                            krys::move(value3))
  {
  }

  ValueList::ValueList(ValueSeparator separator, Ref<Value> value1, Ref<Value> value2, Ref<Value> value3,
                       Ref<Value> value4) noexcept
      : ValueContainingList(ValueType::ValueList, separator, krys::move(value1), krys::move(value2),
                            krys::move(value3), krys::move(value4))
  {
  }

  Ref<ValueList> ValueList::CreateCommaSeparated(ValueListBuilder values) noexcept
  {
    return AdoptRef(*new ValueList(ValueSeparator::CommaSeparator, krys::move(values)));
  }

  Ref<ValueList> ValueList::CreateCommaSeparated(Ref<Value> value) noexcept
  {
    return AdoptRef(*new ValueList(ValueSeparator::CommaSeparator, krys::move(value)));
  }

  Ref<ValueList> ValueList::CreateSlashSeparated(ValueListBuilder values) noexcept
  {
    return AdoptRef(*new ValueList(ValueSeparator::SlashSeparator, krys::move(values)));
  }

  Ref<ValueList> ValueList::CreateSlashSeparated(Ref<Value> value) noexcept
  {
    return AdoptRef(*new ValueList(ValueSeparator::SlashSeparator, krys::move(value)));
  }

  Ref<ValueList> ValueList::CreateSlashSeparated(Ref<Value> value1, Ref<Value> value2) noexcept
  {
    return AdoptRef(*new ValueList(ValueSeparator::SlashSeparator, krys::move(value1), krys::move(value2)));
  }

  Ref<ValueList> ValueList::CreateSpaceSeparated() noexcept
  {
    return AdoptRef(*new ValueList(ValueSeparator::SpaceSeparator));
  }

  Ref<ValueList> ValueList::CreateSpaceSeparated(ValueListBuilder values) noexcept
  {
    return AdoptRef(*new ValueList(ValueSeparator::SpaceSeparator, krys::move(values)));
  }

  Ref<ValueList> ValueList::CreateSpaceSeparated(Ref<Value> value) noexcept
  {
    return AdoptRef(*new ValueList(ValueSeparator::SpaceSeparator, krys::move(value)));
  }

  Ref<ValueList> ValueList::CreateSpaceSeparated(Ref<Value> value1, Ref<Value> value2) noexcept
  {
    return AdoptRef(*new ValueList(ValueSeparator::SpaceSeparator, krys::move(value1), krys::move(value2)));
  }

  Ref<ValueList> ValueList::CreateSpaceSeparated(Ref<Value> value1, Ref<Value> value2,
                                                 Ref<Value> value3) noexcept
  {
    return AdoptRef(*new ValueList(ValueSeparator::SpaceSeparator, krys::move(value1), krys::move(value2),
                                   krys::move(value3)));
  }

  Ref<ValueList> ValueList::CreateSpaceSeparated(Ref<Value> value1, Ref<Value> value2, Ref<Value> value3,
                                                 Ref<Value> value4) noexcept
  {
    return AdoptRef(*new ValueList(ValueSeparator::SpaceSeparator, krys::move(value1), krys::move(value2),
                                   krys::move(value3), krys::move(value4)));
  }

  Ref<ValueList> ValueList::Create(char32 separator, ValueListBuilder builder) noexcept
  {
    switch (separator)
    {
      case U',': return CreateCommaSeparated(krys::move(builder));
      case U'/': return CreateSlashSeparated(krys::move(builder));
      case U' ': return CreateSpaceSeparated(krys::move(builder));
      default:   break;
    }
    krys_unreachable();
  }

  bool ValueContainingList::HasValue(const Value &otherValue) const noexcept
  {
    for (auto &value : *this)
    {
      if (value.Equals(otherValue))
      {
        return true;
      }
    }
    return false;
  }

  bool ValueContainingList::HasValue(css::ValueId otherValue) const noexcept
  {
    for (auto &value : *this)
    {
      if (css::IsValueId(value, otherValue))
      {
        return true;
      }
    }
    return false;
  }

  ValueListBuilder ValueContainingList::CopyValues() const noexcept
  {
    ValueListBuilder result;
    result.reserve(size());
    for (auto &value : *this)
    {
      result.push_back(Ref<Value>::WithRef(const_cast<Value &>(value)));
    }
    return result;
  }

  void ValueContainingList::SerialiseItems(StringBuilder &builder,
                                           const SerialisationContext &context) const noexcept
  {
    bool first = true;
    for (auto &value : *this)
    {
      if (!first)
      {
        builder.Append(SeparatorCSSText());
      }
      first = false;
      builder.Append(value.CssText(context));
    }
  }

  CSSOMString ValueContainingList::SerialiseItems(const SerialisationContext &context) const noexcept
  {
    StringBuilder result;
    SerialiseItems(result, context);
    return krys::move(result).ToString();
  }

  CSSOMString ValueList::CustomCSSText(const SerialisationContext &context) const noexcept
  {
    return SerialiseItems(context);
  }

  bool ValueContainingList::ItemsEqual(const ValueContainingList &other) const noexcept
  {
    uint32 size = this->size();
    if (size != other.size())
    {
      return false;
    }
    for (uint32 i = 0u; i < size; ++i)
    {
      if (!(*this)[i].Equals(other[i]))
      {
        return false;
      }
    }
    return true;
  }

  bool ValueList::Equals(const ValueList &other) const noexcept
  {
    return Separator() == other.Separator() && ItemsEqual(other);
  }

  bool ValueContainingList::ContainsSingleEqualItem(const Value &other) const noexcept
  {
    return size() == 1u && (*this)[0u].Equals(other);
  }

  bool ValueContainingList::AddDerivedHash(size_t &seed) const noexcept
  {
    seed = Hash::Combine(seed, Separator());

    for (auto &item : *this)
    {
      if (!item.AddHash(seed))
      {
        return false;
      }
    }
    return true;
  }

  bool ValueContainingList::CustomTraverseSubresources(
    KRYS_NOESCAPE const Func<bool(const CachedResource &)> &handler) const noexcept
  {
    for (auto &value : *this)
    {
      if (value.TraverseSubresources(handler))
      {
        return true;
      }
    }
    return false;
  }

  IterationStatus ValueContainingList::CustomVisitChildren(
    KRYS_NOESCAPE const Func<IterationStatus(Value &)> &func) const noexcept
  {
    for (auto &value : *this)
    {
      if (func(const_cast<Value &>(value)) == IterationStatus::Done)
      {
        return IterationStatus::Done;
      }
    }
    return IterationStatus::Continue;
  }
}
