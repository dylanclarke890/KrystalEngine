#include "Krystal.Booey/CSS/Values/ValueList.hpp"
#include "Krystal.Booey/CSS/Values/PrimitiveValue.hpp"
#include "Krystal.Core/Hashing/FastHash.hpp"
#include "Krystal.Core/Strings/StringBuilder.hpp"
#include <algorithm>
#include <limits>

namespace krys::boo::css
{
#pragma region ValueContainingList

  ValueContainingList::ValueContainingList(ValueType type, ValueSeparator separator,
                                           ValueListBuilder values) noexcept
      : Value(type), _values(krys::move(values))
  {
    // size() and indexing are in uint32.
    krys_assert(_values.size() <= std::numeric_limits<uint32>::max());
    _header.Separator = separator;
  }

  bool ValueContainingList::HasValue(const Value &value) const noexcept
  {
    return std::ranges::any_of(*this, [&](const Value &item) { return item == value; });
  }

  bool ValueContainingList::HasValue(css::ValueId keyword) const noexcept
  {
    return std::ranges::any_of(*this, [&](const Value &item) { return item.ValueId() == keyword; });
  }

  void ValueContainingList::SerialiseItems(StringBuilder &builder,
                                           const SerialisationContext &context) const noexcept
  {
    bool isFirst = true;
    for (auto &value : *this)
    {
      if (!isFirst)
      {
        builder.Append(ToCssText(Separator()));
      }
      isFirst = false;
      builder.Append(value.CssText(context));
    }
  }

  CSSOMString ValueContainingList::SerialiseItems(const SerialisationContext &context) const noexcept
  {
    StringBuilder builder;
    SerialiseItems(builder, context);
    return krys::move(builder).ToString();
  }

  bool ValueContainingList::ItemsEqual(const ValueContainingList &other) const noexcept
  {
    return std::ranges::equal(*this, other);
  }

  bool ValueContainingList::ContainsSingleEqualItem(const Value &other) const noexcept
  {
    return size() == 1u && (*this)[0u] == other;
  }

  IterationStatus ValueContainingList::CustomVisitChildren(
    KRYS_NOESCAPE const Func<IterationStatus(Value &)> &func) const noexcept
  {
    for (auto &value : _values)
    {
      if (func(*value) == IterationStatus::Done)
      {
        return IterationStatus::Done;
      }
    }
    return IterationStatus::Continue;
  }

  bool ValueContainingList::CustomTraverseSubresources(
    KRYS_NOESCAPE const Func<bool(const CachedResource &)> &handler) const noexcept
  {
    return std::ranges::any_of(*this,
                               [&](const Value &value) { return value.TraverseSubresources(handler); });
  }

  bool ValueContainingList::AddDerivedHash(size_t &seed) const noexcept
  {
    seed = CombineFastHashes(seed, Separator());
    return std::ranges::all_of(*this, [&](const Value &value) { return value.AddHash(seed); });
  }

#pragma endregion

#pragma region ValueList

  ValueList::ValueList(ValueSeparator separator, ValueListBuilder values) noexcept
      : ValueContainingList(ValueType::ValueList, separator, krys::move(values))
  {
  }

  Ref<ValueList> ValueList::Create(ValueSeparator separator, ValueListBuilder values) noexcept
  {
    return AdoptRef(*new ValueList(separator, krys::move(values)));
  }

  Ref<ValueList> ValueList::CreateSpaceSeparated(ValueListBuilder values) noexcept
  {
    return Create(ValueSeparator::Space, krys::move(values));
  }

  Ref<ValueList> ValueList::CreateCommaSeparated(ValueListBuilder values) noexcept
  {
    return Create(ValueSeparator::Comma, krys::move(values));
  }

  Ref<ValueList> ValueList::CreateSlashSeparated(ValueListBuilder values) noexcept
  {
    return Create(ValueSeparator::Slash, krys::move(values));
  }

  CSSOMString ValueList::CustomCSSText(const SerialisationContext &context) const noexcept
  {
    return SerialiseItems(context);
  }

  bool ValueList::Equals(const ValueList &other) const noexcept
  {
    return Separator() == other.Separator() && ItemsEqual(other);
  }

#pragma endregion
}
