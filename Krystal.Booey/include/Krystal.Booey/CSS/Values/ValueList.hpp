#pragma once

#include "Krystal.Booey/CSS/Values/Value.hpp"
#include "Krystal.Booey/CSS/Values/ValueListBuilder.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/SmallList.hpp"
#include <iterator>
#include <ranges>

namespace krys::boo::css
{
  /// @brief Base for values that hold a list of values (`ValueList`, `FunctionValue`, ...).
  class ValueContainingList : public Value
  {
    friend class Value;

  private:
    SmallList<const Value *, ValueListBuilderInlineCapacity> _storage;

  protected:
    ValueContainingList(ValueType type, ValueSeparator separator) noexcept;
    ValueContainingList(ValueType type, ValueSeparator separator, ValueListBuilder values) noexcept;
    ValueContainingList(ValueType type, ValueSeparator separator, Ref<Value> value) noexcept;
    ValueContainingList(ValueType type, ValueSeparator separator, Ref<Value> value1,
                        Ref<Value> value2) noexcept;
    ValueContainingList(ValueType type, ValueSeparator separator, Ref<Value> value1, Ref<Value> value2,
                        Ref<Value> value3) noexcept;
    ValueContainingList(ValueType type, ValueSeparator separator, Ref<Value> value1, Ref<Value> value2,
                        Ref<Value> value3, Ref<Value> value4) noexcept;
    ~ValueContainingList() noexcept;

  public:
    struct iterator
    {
      using iterator_category = std::forward_iterator_tag;
      using value_type = const Value &;
      using difference_type = ptrdiff_t;
      using pointer = const Value *;
      using reference = const Value &;

      KRYS_NODISCARD const Value &operator*() const noexcept
      {
        return (*List)[Index];
      }

      iterator &operator++() noexcept
      {
        ++Index;
        return *this;
      }

      iterator operator++(int) noexcept
      {
        auto result = *this;
        ++*this;
        return result;
      }

      KRYS_NODISCARD constexpr bool operator==(const iterator &other) const noexcept
      {
        return Index == other.Index;
      }

      const ValueContainingList *List {nullptr};
      uint32 Index {0u};
    };

    using const_iterator = iterator;

    KRYS_NODISCARD uint32 size() const noexcept
    {
      return static_cast<uint32>(_storage.size());
    }

    KRYS_NODISCARD const Value &operator[](uint32 index) const noexcept KRYS_LIFETIME_BOUND
    {
      krys_debug_assert(index < size());
      return *_storage[index];
    }

    KRYS_NODISCARD iterator begin() const noexcept KRYS_LIFETIME_BOUND
    {
      return {this, 0u};
    }

    KRYS_NODISCARD iterator end() const noexcept KRYS_LIFETIME_BOUND
    {
      return {this, size()};
    }

    KRYS_NODISCARD bool HasValue(const Value &value) const noexcept;
    KRYS_NODISCARD bool HasValue(css::ValueId value) const noexcept;

    void SerialiseItems(StringBuilder &builder, const SerialisationContext &context) const noexcept;
    KRYS_NODISCARD CSSOMString SerialiseItems(const SerialisationContext &context) const noexcept;

    KRYS_NODISCARD bool ItemsEqual(const ValueContainingList &other) const noexcept;
    KRYS_NODISCARD bool ContainsSingleEqualItem(const Value &other) const noexcept;

    using Value::Separator;
    using Value::SeparatorCSSText;

    KRYS_NODISCARD bool CustomTraverseSubresources(
      KRYS_NOESCAPE const Func<bool(const CachedResource &)> &handler) const noexcept;

    KRYS_NODISCARD ValueListBuilder CopyValues() const noexcept;

    // Consider removing these functions and having callers use size() and operator[] instead.
    KRYS_NODISCARD uint32 Length() const noexcept
    {
      return size();
    }

    KRYS_NODISCARD const Value *Item(uint32 index) const noexcept KRYS_LIFETIME_BOUND
    {
      return index < size() ? &(*this)[index] : nullptr;
    }

    KRYS_NODISCARD RefPtr<const Value> ProtectedItem(uint32 index) const noexcept
    {
      return RefPtr<const Value>::WithRef(Item(index));
    }

    KRYS_NODISCARD const Value *ItemWithoutBoundsCheck(uint32 index) const noexcept KRYS_LIFETIME_BOUND
    {
      return &(*this)[index];
    }

    KRYS_NODISCARD IterationStatus
      CustomVisitChildren(KRYS_NOESCAPE const Func<IterationStatus(Value &)> &func) const noexcept;

  protected:
    KRYS_NODISCARD bool AddDerivedHash(size_t &seed) const noexcept;
  };

  static_assert(std::ranges::range<ValueContainingList>);

  /// @brief A list of values separated by spaces, commas or slashes.
  class ValueList final : public ValueContainingList
  {
  private:
    explicit ValueList(ValueSeparator separator) noexcept;
    ValueList(ValueSeparator separator, ValueListBuilder values) noexcept;
    ValueList(ValueSeparator separator, Ref<Value> value) noexcept;
    ValueList(ValueSeparator separator, Ref<Value> value1, Ref<Value> value2) noexcept;
    ValueList(ValueSeparator separator, Ref<Value> value1, Ref<Value> value2, Ref<Value> value3) noexcept;
    ValueList(ValueSeparator separator, Ref<Value> value1, Ref<Value> value2, Ref<Value> value3,
              Ref<Value> value4) noexcept;

  public:
    /// @param separator `','`, `'/'` or `' '`.
    KRYS_NODISCARD static Ref<ValueList> Create(char32 separator, ValueListBuilder values) noexcept;

    KRYS_NODISCARD static Ref<ValueList> CreateCommaSeparated(ValueListBuilder values) noexcept;
    // FIXME: Upgrade callers to not use a list at all.
    KRYS_NODISCARD static Ref<ValueList> CreateCommaSeparated(Ref<Value> value) noexcept;

    KRYS_NODISCARD static Ref<ValueList> CreateSpaceSeparated(ValueListBuilder values) noexcept;
    // FIXME: Get rid of the caller that needs to create an empty list.
    KRYS_NODISCARD static Ref<ValueList> CreateSpaceSeparated() noexcept;
    // FIXME: Upgrade callers to not use a list at all.
    KRYS_NODISCARD static Ref<ValueList> CreateSpaceSeparated(Ref<Value> value) noexcept;
    KRYS_NODISCARD static Ref<ValueList> CreateSpaceSeparated(Ref<Value> value1, Ref<Value> value2) noexcept;
    KRYS_NODISCARD static Ref<ValueList> CreateSpaceSeparated(Ref<Value> value1, Ref<Value> value2,
                                                              Ref<Value> value3) noexcept;
    KRYS_NODISCARD static Ref<ValueList> CreateSpaceSeparated(Ref<Value> value1, Ref<Value> value2,
                                                              Ref<Value> value3, Ref<Value> value4) noexcept;

    KRYS_NODISCARD static Ref<ValueList> CreateSlashSeparated(ValueListBuilder values) noexcept;
    // FIXME: Upgrade callers to not use a list at all.
    KRYS_NODISCARD static Ref<ValueList> CreateSlashSeparated(Ref<Value> value) noexcept;
    KRYS_NODISCARD static Ref<ValueList> CreateSlashSeparated(Ref<Value> value1, Ref<Value> value2) noexcept;

    KRYS_NODISCARD CSSOMString CustomCSSText(const SerialisationContext &context) const noexcept;
    KRYS_NODISCARD bool Equals(const ValueList &other) const noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::ValueContainingList)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::Value &value) noexcept
  {
    return value.ContainsValueList();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::ValueList)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::Value &value) noexcept
  {
    return value.IsValueListValue();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
