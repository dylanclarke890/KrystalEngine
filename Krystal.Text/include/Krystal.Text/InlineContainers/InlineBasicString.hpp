#pragma once

#include "Krystal.Lib/String/EmptyString.hpp"
#include "Krystal.Text/InlineContainers/InlineVector.hpp"
#include "Krystal.Text/TypeTraits.hpp"
#include <algorithm>
#include <functional>
#include <string_view>

namespace Krys::Text
{
  namespace Impl
  {
    template <typename T, typename TSelf, bool = IsCharTraitable<T>>
    class SVConversion
    {
    };

    template <typename T, typename TSelf>
    class SVConversion<T, TSelf, true>
    {
    public:
      template <typename TTraits = std::char_traits<T>>
      constexpr operator std::basic_string_view<T, TTraits>() const noexcept
      {
        const TSelf &self = static_cast<const TSelf &>(*this);
        return std::basic_string_view<T, TTraits>(self.data(), self.size());
      }
    };
  }

  template <typename T, std::size_t MaxLength>
  class InlineBasicString : private InlineVector<T, MaxLength + 1>,
                            public Impl::SVConversion<T, InlineBasicString<T, MaxLength>>
  {
  private:
    using TBase = InlineVector<T, MaxLength + 1>;

    constexpr void SetNullTerminator() noexcept
    {
      auto *sentinelPtr = this->Data(this->_layout.Size);
      std::construct_at(sentinelPtr);
    }

  public:
    using value_type = typename TBase::value_type;
    using reference = typename TBase::reference;
    using const_reference = typename TBase::const_reference;
    using pointer = typename TBase::pointer;
    using const_pointer = typename TBase::const_pointer;
    using iterator = typename TBase::iterator;
    using const_iterator = typename TBase::const_iterator;
    using reverse_iterator = typename TBase::reverse_iterator;
    using const_reverse_iterator = typename TBase::const_reverse_iterator;
    using size_type = typename TBase::size_type;
    using difference_type = typename TBase::difference_type;
    constexpr inline static const std::size_t inline_capacity = MaxLength;

    constexpr InlineBasicString() noexcept : TBase()
    {
      this->SetNullTerminator();
    }

    constexpr InlineBasicString(std::initializer_list<value_type> values) noexcept : TBase(values)
    {
      this->SetNullTerminator();
    }

    constexpr InlineBasicString(size_type count) noexcept : TBase(count)
    {
      this->SetNullTerminator();
    }

    constexpr InlineBasicString(size_type count, const value_type &value) noexcept : TBase(count, value)
    {
      this->SetNullTerminator();
    }

    template <typename TFirst, typename TLast>
    requires(!Integral<TFirst> && !SameType<remove_cvref_t<TLast>, value_type>)
    constexpr InlineBasicString(TFirst first, TLast last) noexcept : TBase(std::move(first), std::move(last))
    {
      this->SetNullTerminator();
    }

    template <typename... TArgs>
    constexpr reference emplace_back(TArgs &&...args) noexcept
    {
      reference val = this->TBase::emplace_back(std::forward<TArgs>(args)...);
      this->SetNullTerminator();
      return val;
    }

    constexpr reference push_back(const value_type &value) noexcept
    {
      reference val = this->TBase::push_back(value);
      this->SetNullTerminator();
      return val;
    }

    constexpr reference push_back(value_type &&value) noexcept
    {
      reference val = this->TBase::push_back(std::move(value));
      this->SetNullTerminator();
      return val;
    }

    template <typename... TArgs>
    constexpr reference emplace_front(TArgs &&...args) noexcept
    {
      reference val = this->TBase::emplace_front(std::forward<TArgs>(args)...);
      this->SetNullTerminator();
      return val;
    }

    constexpr reference push_front(const value_type &value) noexcept
    {
      reference val = this->TBase::push_front(value);
      this->SetNullTerminator();
      return val;
    }

    constexpr reference push_front(value_type &&value) noexcept
    {
      reference val = this->TBase::push_front(std::move(value));
      this->SetNullTerminator();
      return val;
    }

    template <typename... TArgs>
    constexpr iterator emplace(const_iterator where, TArgs &&...args) noexcept
    {
      iterator itVal = this->TBase::emplace(std::move(where), std::forward<TArgs>(args)...);
      this->SetNullTerminator();
      return itVal;
    }

    constexpr iterator insert(const_iterator where, value_type &&value) noexcept
    {
      iterator itVal = this->TBase::insert(std::move(where), std::move(value));
      this->SetNullTerminator();
      return itVal;
    }

    constexpr iterator insert(const_iterator where, const value_type &value) noexcept
    {
      iterator itVal = this->TBase::insert(std::move(where), value);
      this->SetNullTerminator();
      return itVal;
    }

    constexpr iterator insert(const_iterator where, std::initializer_list<value_type> __ilvalue) noexcept
    {
      iterator itVal = this->TBase::insert(std::move(where), __ilvalue);
      this->SetNullTerminator();
      return itVal;
    }

    constexpr iterator insert(const_iterator where, size_type count, const value_type &value) noexcept
    {
      iterator itVal = this->TBase::insert(std::move(where), count, value);
      this->SetNullTerminator();
      return itVal;
    }

    template <typename TFirst, typename TLast>
    requires(!Integral<TFirst> && !SameType<remove_cvref_t<TLast>, value_type>)
    constexpr iterator insert(const_iterator where, TFirst first, TLast last)
    {
      iterator itVal = this->TBase::insert(std::move(where), std::move(first), std::move(last));
      this->SetNullTerminator();
      return itVal;
    }

    constexpr iterator erase(const_iterator where) noexcept
    {
      iterator itVal = this->TBase::erase(std::move(where));
      this->SetNullTerminator();
      return itVal;
    }

    constexpr iterator erase(const_iterator whereFrom, const_iterator whereTo) noexcept
    {
      iterator itVal = this->TBase::erase(std::move(whereFrom), std::move(whereTo));
      this->SetNullTerminator();
      return itVal;
    }

    using TBase::operator[];
    using TBase::back;
    using TBase::begin;
    using TBase::cbegin;
    using TBase::cend;
    using TBase::clear;
    using TBase::crbegin;
    using TBase::crend;
    using TBase::data;
    using TBase::empty;
    using TBase::end;
    using TBase::front;
    using TBase::is_empty;
    using TBase::rbegin;
    using TBase::rend;
    using TBase::size;

    constexpr const_pointer c_str() const noexcept
    {
      return this->data();
    }

    constexpr size_type max_size() const noexcept
    {
      return MaxLength;
    }

    constexpr size_type capacity() const noexcept
    {
      return MaxLength;
    }
  };

  template <typename T>
  class InlineBasicString<T, 0>
  {
  public:
    using value_type = T;
    using reference = std::add_lvalue_reference_t<T>;
    using const_reference = std::add_lvalue_reference_t<const T>;
    using pointer = std::add_pointer_t<T>;
    using const_pointer = std::add_pointer_t<const T>;
    using iterator = Krys::Ranges::WrappedPointer<T>;
    using const_iterator = Krys::Ranges::WrappedPointer<const T>;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    constexpr inline static const std::size_t inline_capacity = 0;

    constexpr InlineBasicString() noexcept
    {
    }

    constexpr InlineBasicString(std::initializer_list<value_type> values) noexcept
    {
      assert(values.size() < size_type(1) && "there must be no values");
    }

    constexpr InlineBasicString(size_type count) noexcept
    {
      assert(count < size_type(1) && "there must be no values");
    }

    template <typename... TArgs>
    reference emplace_back(TArgs &&...) noexcept
    {
      assert(false && "there is no capacity in this InlineBasicString");
      return *static_cast<pointer>(nullptr);
    }

    reference push_back(const value_type &) noexcept
    {
      assert(false && "there is no capacity in this InlineBasicString");
      return *static_cast<pointer>(nullptr);
    }

    reference push_back(value_type &&) noexcept
    {
      assert(false && "there is no capacity in this InlineBasicString");
      return *static_cast<pointer>(nullptr);
    }

    template <typename... TArgs>
    reference emplace_front(TArgs &&...) noexcept
    {
      assert(false && "there is no capacity in this InlineBasicString");
      return *static_cast<pointer>(nullptr);
    }

    reference push_front(const value_type &) noexcept
    {
      assert(false && "there is no capacity in this InlineBasicString");
      return *static_cast<pointer>(nullptr);
    }

    reference push_front(value_type &&) noexcept
    {
      assert(false && "there is no capacity in this InlineBasicString");
      return *static_cast<pointer>(nullptr);
    }

    template <typename... TArgs>
    iterator emplace(const_iterator, TArgs &&...) noexcept
    {
      assert(false && "nothing can be emplaced in this InlineBasicString");
      return iterator(nullptr);
    }

    constexpr iterator insert(const_iterator, value_type &&) noexcept
    {
      assert(false && "nothing can be inserted in this InlineBasicString");
      return iterator(nullptr);
    }

    constexpr iterator insert(const_iterator, const value_type &) noexcept
    {
      assert(false && "nothing can be inserted in this InlineBasicString");
      return iterator(nullptr);
    }

    constexpr iterator insert(const_iterator, std::initializer_list<value_type>) noexcept
    {
      assert(false && "nothing can be inserted in this InlineBasicString");
      return iterator(nullptr);
    }

    constexpr iterator insert(const_iterator, size_type) noexcept
    {
      assert(false && "nothing can be inserted in this InlineBasicString");
      return iterator(nullptr);
    }

    constexpr iterator insert(const_iterator, size_type, const value_type &) noexcept
    {
      assert(false && "nothing can be inserted in this InlineBasicString");
      return iterator(nullptr);
    }

    template <typename TFirst, typename TLast>
    requires(!Integral<TFirst> && !SameType<remove_cvref_t<TLast>, value_type>)
    constexpr iterator insert(const_iterator, TFirst, TLast)
    {
      assert(false && "there is no capacity in this InlineBasicString");
      return iterator(nullptr);
    }

    constexpr iterator erase(const_iterator) noexcept
    {
      assert(false && "there is nothing to erase in this InlineBasicString");
      return iterator(nullptr);
    }

    constexpr iterator erase(const_iterator, const_iterator) noexcept
    {
      assert(false && "there is nothing to erase in this InlineBasicString");
      return iterator(nullptr);
    }

    constexpr void pop_back() noexcept
    {
      assert(false && "there is nothing to erase in this InlineBasicString");
    }

    constexpr void pop_front() noexcept
    {
      assert(false && "there is nothing to erase in this InlineBasicString");
    }

    constexpr const_reference front() const noexcept
    {
#if 0
			assert(false && "there is no capacity in this InlineBasicString");
#endif
      return *Krys::EmptyString<value_type>();
    }

    constexpr const_reference back() const noexcept
    {
#if 0
			assert(false && "there is no capacity in this InlineBasicString");
#endif
      return *Krys::EmptyString<value_type>();
    }

    constexpr bool empty() const noexcept
    {
      return true;
    }

    constexpr void clear() noexcept
    {
    }

    constexpr size_type size() const noexcept
    {
      return 0;
    }

    constexpr size_type capacity() const noexcept
    {
      return 0;
    }

    constexpr size_type max_size() const noexcept
    {
      return 0;
    }

    constexpr const_pointer data() const noexcept
    {
      return Krys::EmptyString<value_type>();
    }

    constexpr const_pointer c_str() const noexcept
    {
      return Krys::EmptyString<value_type>();
    }

    constexpr const_iterator begin() const noexcept
    {
      return const_iterator(Krys::EmptyString<value_type>() + 0);
    }

    constexpr const_iterator cbegin() const noexcept
    {
      return const_iterator(Krys::EmptyString<value_type>() + 0);
    }

    constexpr const_iterator end() const noexcept
    {
      return const_iterator(Krys::EmptyString<value_type>() + 0);
    }

    constexpr const_iterator cend() const noexcept
    {
      return const_iterator(Krys::EmptyString<value_type>() + 0);
    }

    constexpr reference operator[](size_type)
    {
      assert(false && "there is no capacity in this InlineBasicString");
      return *static_cast<pointer>(nullptr);
    }

    constexpr const_reference operator[](size_type) const
    {
      assert(false && "there is no capacity in this InlineBasicString");
      return *static_cast<const_pointer>(nullptr);
    }

    constexpr void resize(size_type count) noexcept
    {
      assert(count < size_type(1)
             && "there is no capacity in this InlineBasicString to resize to anything but 0");
    }

    constexpr void resize(size_type count, const value_type &) noexcept
    {
      assert(count < size_type(1)
             && "there is no capacity in this InlineBasicString to resize to anything but 0");
    }
  };

  template <typename TLeft, std::size_t LeftMaxLength, typename TRight, std::size_t RightMaxLength>
  constexpr bool operator==(const InlineBasicString<TLeft, LeftMaxLength> &left,
                            const InlineBasicString<TRight, RightMaxLength> &right) noexcept
  {
    return std::equal(left.cbegin(), left.cend(), right.cbegin(), right.cend());
  }

  template <typename TLeft, std::size_t LeftMaxLength, typename TRight, std::size_t RightMaxLength>
  constexpr bool operator!=(const InlineBasicString<TLeft, LeftMaxLength> &left,
                            const InlineBasicString<TRight, RightMaxLength> &right) noexcept
  {
    return !std::equal(left.cbegin(), left.cend(), right.cbegin(), right.cend());
  }

  template <typename TLeft, std::size_t LeftMaxLength, typename TRight, std::size_t RightMaxLength>
  constexpr bool operator<(const InlineBasicString<TLeft, LeftMaxLength> &left,
                           const InlineBasicString<TRight, RightMaxLength> &right) noexcept
  {
    return std::lexicographical_compare(left.cbegin(), left.cend(), right.cbegin(), right.cend(),
                                        std::less_equal<>());
  }

  template <typename TLeft, std::size_t LeftMaxLength, typename TRight, std::size_t RightMaxLength>
  constexpr bool operator>(const InlineBasicString<TLeft, LeftMaxLength> &left,
                           const InlineBasicString<TRight, RightMaxLength> &right) noexcept
  {
    return std::lexicographical_compare(left.cbegin(), left.cend(), right.cbegin(), right.cend(),
                                        std::greater<>());
  }

  template <typename TLeft, std::size_t LeftMaxLength, typename TRight, std::size_t RightMaxLength>
  constexpr bool operator<=(const InlineBasicString<TLeft, LeftMaxLength> &left,
                            const InlineBasicString<TRight, RightMaxLength> &right) noexcept
  {
    return std::lexicographical_compare(left.cbegin(), left.cend(), right.cbegin(), right.cend(),
                                        std::less_equal<>());
  }

  template <typename TLeft, std::size_t LeftMaxLength, typename TRight, std::size_t RightMaxLength>
  constexpr bool operator>=(const InlineBasicString<TLeft, LeftMaxLength> &left,
                            const InlineBasicString<TRight, RightMaxLength> &right) noexcept
  {
    return std::lexicographical_compare(left.cbegin(), left.cend(), right.cbegin(), right.cend(),
                                        std::greater_equal<>());
  }
}