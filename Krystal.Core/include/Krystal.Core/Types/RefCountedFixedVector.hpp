#pragma once

#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Types/FixedVector.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys
{
  /// @brief A reference-counted, fixed-size vector, so that copies of its owners share the elements.
  template <typename T>
  class RefCountedFixedVector : public RefCounted<RefCountedFixedVector<T>>
  {
  public:
    using Container = FixedVector<T>;
    using value_type = typename Container::value_type;
    using const_iterator = typename Container::const_iterator;
    using const_reverse_iterator = typename Container::const_reverse_iterator;

  private:
    Container _values;

  public:
    KRYS_NODISCARD static Ref<RefCountedFixedVector> create(T &&value)
    {
      return AdoptRef(*new RefCountedFixedVector(Container {std::move(value)}));
    }

    KRYS_NODISCARD static Ref<RefCountedFixedVector> create(std::initializer_list<T> values)
    {
      return AdoptRef(*new RefCountedFixedVector(Container {values}));
    }

    template <std::invocable<size_t> Generator>
    KRYS_NODISCARD static Ref<RefCountedFixedVector> createWithSizeFromGenerator(size_t size,
                                                                                 Generator &&generator)
    {
      return AdoptRef(*new RefCountedFixedVector(
        Container::createWithSizeFromGenerator(size, std::forward<Generator>(generator))));
    }

    template <typename SizedRange, typename Mapper>
    KRYS_NODISCARD static Ref<RefCountedFixedVector> map(SizedRange &&range, Mapper &&mapper)
    {
      return AdoptRef(*new RefCountedFixedVector(
        Container::map(std::forward<SizedRange>(range), std::forward<Mapper>(mapper))));
    }

    KRYS_NODISCARD const_iterator begin() const noexcept KRYS_LIFETIME_BOUND
    {
      return _values.begin();
    }
    KRYS_NODISCARD const_iterator end() const noexcept KRYS_LIFETIME_BOUND
    {
      return _values.end();
    }
    KRYS_NODISCARD const_reverse_iterator rbegin() const noexcept KRYS_LIFETIME_BOUND
    {
      return _values.rbegin();
    }
    KRYS_NODISCARD const_reverse_iterator rend() const noexcept KRYS_LIFETIME_BOUND
    {
      return _values.rend();
    }

    KRYS_NODISCARD bool isEmpty() const noexcept
    {
      return _values.isEmpty();
    }
    KRYS_NODISCARD size_t size() const noexcept
    {
      return _values.size();
    }
    KRYS_NODISCARD const T &operator[](size_t index) const noexcept KRYS_LIFETIME_BOUND
    {
      return _values[index];
    }
    KRYS_NODISCARD const T &first() const noexcept KRYS_LIFETIME_BOUND
    {
      return _values.first();
    }
    KRYS_NODISCARD const T &last() const noexcept KRYS_LIFETIME_BOUND
    {
      return _values.last();
    }

    KRYS_NODISCARD bool operator==(const RefCountedFixedVector &other) const
    {
      return _values == other._values;
    }

  private:
    explicit RefCountedFixedVector(Container &&values) noexcept : _values {std::move(values)}
    {
    }
  };
}
