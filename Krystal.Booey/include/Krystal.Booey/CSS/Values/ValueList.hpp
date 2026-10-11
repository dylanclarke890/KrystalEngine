#pragma once

#include "Krystal.Booey/CSS/Values/Value.hpp"
#include "Krystal.Booey/CSS/Values/ValueListBuilder.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/SmallList.hpp"

namespace krys::boo::css
{
  class ValueWithSmallList : public Value
  {
  protected:
    ValueWithSmallList(ValueType type, ValueSeparator separator) noexcept;

    ValueWithSmallList(ValueType type, ValueSeparator separator, ValueListBuilder values) noexcept;

    ValueWithSmallList(ValueType type, ValueSeparator separator, Ref<Value> value) noexcept;

    ValueWithSmallList(ValueType type, ValueSeparator separator, Ref<Value> first,
                       Ref<Value> second) noexcept;

    ValueWithSmallList(ValueType type, ValueSeparator separator, Ref<Value> first, Ref<Value> second,
                       Ref<Value> third) noexcept;

    ValueWithSmallList(ValueType type, ValueSeparator separator, Ref<Value> first, Ref<Value> second,
                       Ref<Value> third, Ref<Value> fourth) noexcept;

    ~ValueWithSmallList() noexcept
    {
      for (auto value : _storage)
      {
        value->SubRef();
      }
    }

  public:
    struct iterator
    {
      using iterator_category = std::forward_iterator_tag;
      using value_type = const Value &;
      using difference_type = ptrdiff_t;
      using pointer = const Value *;
      using reference = const Value &;

      KRYS_NODISCARD const Value &operator*() const
      {
        return (*List)[Index];
      }

      iterator &operator++()
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

      const ValueWithSmallList *List {nullptr};
      size_t Index {0uz};
    };

    using const_iterator = iterator;

  private:
    SmallList<const Value *, 4uz> _storage;

  public:
    using Value::Separator;

    KRYS_NODISCARD const Value *TryGet(size_t index) const noexcept
    {
      return index < _storage.size() ? _storage[index] : nullptr;
    }

    KRYS_NODISCARD const Value &Get(size_t index) const noexcept
    {
      assert(index < _storage.size());
      return *_storage[index];
    }

    KRYS_NODISCARD size_t Size() const noexcept
    {
      return _storage.size();
    }

    KRYS_NODISCARD iterator begin() const noexcept
    {
      return {this, 0};
    }

    KRYS_NODISCARD iterator end() const noexcept
    {
      return {this, Size()};
    }

    KRYS_NODISCARD bool HasValue(const Value &value) const noexcept;

    KRYS_NODISCARD bool HasValue(css::ValueId id) const noexcept;

    void SerialiseItems(CSSOMString &builder, const SerialisationContext &context) const noexcept;

    KRYS_NODISCARD CSSOMString SerializeItems(const SerialisationContext &context) const noexcept;

    KRYS_NODISCARD bool ItemsEqual(const ValueWithSmallList &items) const noexcept;

    KRYS_NODISCARD bool ContainsSingleEqualItem(const Value &value) const noexcept;

    KRYS_NODISCARD ValueListBuilder CopyValues() const noexcept;

    KRYS_NODISCARD const Value &operator[](size_t index) const noexcept;

    KRYS_NODISCARD IterationStatus
      CustomVisitChildren(const Func<IterationStatus(Value &)> &func) const noexcept;

    KRYS_NODISCARD bool CustomTraverseSubresources(const Func<bool(const CachedResource &)> &func) const;
  };

  class ValueList final : public ValueWithSmallList
  {
  private:
    explicit ValueList(ValueSeparator);
    ValueList(ValueSeparator separator, ValueListBuilder values) noexcept;
    ValueList(ValueSeparator separator, Ref<Value> value) noexcept;
    ValueList(ValueSeparator separator, Ref<Value> first, Ref<Value> second) noexcept;
    ValueList(ValueSeparator separator, Ref<Value> first, Ref<Value> second, Ref<Value> third) noexcept;
    ValueList(ValueSeparator separator, Ref<Value> first, Ref<Value> second, Ref<Value> third,
              Ref<Value> fourth) noexcept;

  public:
    KRYS_NODISCARD static Ref<ValueList> Create(char32 separator, ValueListBuilder &&values) noexcept;

    KRYS_NODISCARD static Ref<ValueList> CreateCommaSeparated(ValueListBuilder &&values) noexcept;

    // FIXME: Upgrade callers to not use a list at all.
    KRYS_NODISCARD static Ref<ValueList> CreateSpaceSeparated(Ref<Value> value) noexcept;

    KRYS_NODISCARD static Ref<ValueList> CreateSpaceSeparated(Ref<Value> first, Ref<Value> second) noexcept;

    KRYS_NODISCARD static Ref<ValueList> CreateSpaceSeparated(Ref<Value> first, Ref<Value> second,
                                                              Ref<Value> third) noexcept;

    KRYS_NODISCARD static Ref<ValueList> CreateSpaceSeparated(Ref<Value> first, Ref<Value> second,
                                                              Ref<Value> third, Ref<Value> fourth) noexcept;

    KRYS_NODISCARD static Ref<ValueList> CreateSpaceSeparated(ValueListBuilder &&values) noexcept;

    KRYS_NODISCARD static Ref<ValueList> CreateSlashSeparated(ValueListBuilder &&values) noexcept;

    KRYS_NODISCARD static Ref<ValueList> CreateSlashSeparated(Ref<Value> first, Ref<Value> second) noexcept;

    KRYS_NODISCARD CSSOMString CustomCSSText(const SerialisationContext &context) const noexcept;

    KRYS_NODISCARD bool Equals(const ValueList &other) const noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::ValueWithSmallList)
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