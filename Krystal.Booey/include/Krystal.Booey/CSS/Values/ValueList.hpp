#pragma once

#include "Krystal.Booey/CSS/Values/Value.hpp"
#include "Krystal.Booey/CSS/Values/ValueListBuilder.hpp"
#include "Krystal.Core/Concepts.hpp"
#include "Krystal.Core/Memory/RefPtr.hpp"
#include <iterator>
#include <ranges>
#include <utility>

namespace krys
{
  class StringBuilder;
}

namespace krys::boo::css
{
  /// @brief The base of the values made of a sequence of values with a separator between them: `ValueList`,
  /// `FunctionValue`'s arguments, `TransformListValue` and the grid and image-set lists.
  /// @details It's a read-only range of `const Value &`.
  class ValueContainingList : public Value
  {
    friend class Value;

  private:
    ValueListBuilder _values;

  public:
    /// @brief Walks the values in order.
    struct iterator
    {
      using iterator_category = std::forward_iterator_tag;
      using value_type = Value;
      using difference_type = ptrdiff_t;
      using pointer = const Value *;
      using reference = const Value &;

      const Ref<Value> *Position {nullptr};

      KRYS_NODISCARD const Value &operator*() const noexcept
      {
        return **Position;
      }

      iterator &operator++() noexcept
      {
        ++Position;
        return *this;
      }

      iterator operator++(int) noexcept
      {
        auto previous = *this;
        ++Position;
        return previous;
      }

      KRYS_NODISCARD constexpr bool operator==(const iterator &other) const noexcept = default;
    };

    using const_iterator = iterator;

    KRYS_NODISCARD uint32 size() const noexcept
    {
      return static_cast<uint32>(_values.size());
    }

    /// @pre @p index < `size()`.
    KRYS_NODISCARD const Value &operator[](uint32 index) const noexcept KRYS_LIFETIME_BOUND
    {
      krys_debug_assert(index < size());
      return *_values[index];
    }

    KRYS_NODISCARD iterator begin() const noexcept KRYS_LIFETIME_BOUND
    {
      return {_values.data()};
    }

    KRYS_NODISCARD iterator end() const noexcept KRYS_LIFETIME_BOUND
    {
      return {_values.data() + _values.size()};
    }

    using Value::Separator;

    /// @brief Whether one of the values equals @p value.
    KRYS_NODISCARD bool HasValue(const Value &value) const noexcept;

    /// @brief Whether one of the values is the keyword @p keyword.
    KRYS_NODISCARD bool HasValue(css::ValueId keyword) const noexcept;

    /// @brief A builder holding the same values, to make a changed copy from.
    KRYS_NODISCARD ValueListBuilder CopyValues() const noexcept
    {
      return _values;
    }

    /// @brief Appends the values' text to @p builder, with the separator between them.
    void SerialiseItems(StringBuilder &builder, const SerialisationContext &context) const noexcept;

    /// @brief The values' text, with the separator between them.
    KRYS_NODISCARD CSSOMString SerialiseItems(const SerialisationContext &context) const noexcept;

    /// @brief Whether both hold equal values in the same order; the separators aren't compared.
    KRYS_NODISCARD bool ItemsEqual(const ValueContainingList &other) const noexcept;

    /// @brief Whether this holds exactly one value, equal to @p other.
    KRYS_NODISCARD bool ContainsSingleEqualItem(const Value &other) const noexcept;

    KRYS_NODISCARD IterationStatus
      CustomVisitChildren(KRYS_NOESCAPE const Func<IterationStatus(Value &)> &func) const noexcept;

    KRYS_NODISCARD bool CustomTraverseSubresources(
      KRYS_NOESCAPE const Func<bool(const CachedResource &)> &handler) const noexcept;

  protected:
    ValueContainingList(ValueType type, ValueSeparator separator, ValueListBuilder values) noexcept;

    /// @brief Holds @p values, of which there may be none.
    template <ConvertibleTo<Ref<Value>>... TValues>
    ValueContainingList(ValueType type, ValueSeparator separator, TValues &&...values) noexcept
        : ValueContainingList(type, separator, MakeBuilder(std::forward<TValues>(values)...))
    {
    }

    /// @brief A builder holding @p values.
    template <ConvertibleTo<Ref<Value>>... TValues>
    KRYS_NODISCARD static ValueListBuilder MakeBuilder(TValues &&...values) noexcept
    {
      ValueListBuilder builder;
      builder.reserve(sizeof...(values));
      (builder.push_back(Ref<Value>(std::forward<TValues>(values))), ...);
      return builder;
    }

    KRYS_NODISCARD bool AddDerivedHash(size_t &seed) const noexcept;
  };

  static_assert(std::ranges::forward_range<ValueContainingList>);

  /// @brief A list of values separated by spaces, commas or slashes, such as the layers of
  /// `background-image` or the parts of `border-image-slice / width`.
  /// @see https://www.w3.org/TR/css-values-4/#component-combinators
  class ValueList final : public ValueContainingList
  {
  private:
    ValueList(ValueSeparator separator, ValueListBuilder values) noexcept;

  public:
    KRYS_NODISCARD static Ref<ValueList> Create(ValueSeparator separator, ValueListBuilder values) noexcept;

    KRYS_NODISCARD static Ref<ValueList> CreateSpaceSeparated(ValueListBuilder values) noexcept;

    KRYS_NODISCARD static Ref<ValueList> CreateCommaSeparated(ValueListBuilder values) noexcept;

    KRYS_NODISCARD static Ref<ValueList> CreateSlashSeparated(ValueListBuilder values) noexcept;

    /// @brief A space-separated list of the values given.
    template <ConvertibleTo<Ref<Value>>... TValues>
    requires(sizeof...(TValues) >= 2uz)
    KRYS_NODISCARD static Ref<ValueList> CreateSpaceSeparated(TValues &&...values) noexcept
    {
      return CreateSpaceSeparated(MakeBuilder(std::forward<TValues>(values)...));
    }

    /// @brief A slash-separated list of the values given.
    template <ConvertibleTo<Ref<Value>>... TValues>
    requires(sizeof...(TValues) >= 2uz)
    KRYS_NODISCARD static Ref<ValueList> CreateSlashSeparated(TValues &&...values) noexcept
    {
      return CreateSlashSeparated(MakeBuilder(std::forward<TValues>(values)...));
    }

    KRYS_NODISCARD CSSOMString CustomCSSText(const SerialisationContext &context) const noexcept;

    /// @brief Whether both have the same separator and equal values in the same order.
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
    return value.Type() == krys::boo::css::ValueType::ValueList;
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
