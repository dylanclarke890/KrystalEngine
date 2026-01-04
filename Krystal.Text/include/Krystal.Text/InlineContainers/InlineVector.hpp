#pragma once

#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Core/TypeTraits.hpp"
#include "Krystal.Lib/Ranges/WrappedPointer.hpp"
#include "Krystal.Lib/Utils/ConstructDestroy.hpp"
#include "Krystal.Lib/Utils/ToAddress.hpp"
#include "Krystal.Lib/Utils/ToMutableIter.hpp"
#include "Krystal.Text/InlineContainers/detail/Storage.hpp"
#include "Krystal.Text/InlineContainers/Forward.hpp"
#include <cassert>
#include <cstddef>
#include <initializer_list>
#include <iterator>
#include <memory>
#include <type_traits>

namespace Krys
{
  template <typename T, std::size_t Capacity>
  class InlineVector : private detail::Storage<T, Capacity>
  {
  private:
    template <typename, std::size_t>
    friend class InlineBasicString;

    using TBaseStorage = detail::Storage<T, Capacity>;

  public:
    using value_type = T;
    using reference = add_lvalue_ref_t<T>;
    using const_reference = add_lvalue_ref_t<const T>;
    using pointer = add_pointer_t<T>;
    using const_pointer = add_pointer_t<const T>;
    using iterator = Krys::Ranges::WrappedPointer<typename TBaseStorage::storage_type>;
    using const_iterator = Krys::Ranges::WrappedPointer<const typename TBaseStorage::storage_type>;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    inline static constexpr const std::size_t inline_capacity = Capacity;

  private:
    template <typename... TArgs>
    static constexpr bool IsEmplaceNoexcept() noexcept
    {
      return NoThrowConstructible<value_type, TArgs...> // cf
             && (NoThrowMoveAssignable<value_type>      // cf
                 || NoThrowCopyAssignable<value_type>);
    }

  public:
    constexpr InlineVector() noexcept : TBaseStorage()
    {
    }

    constexpr InlineVector(std::initializer_list<value_type> values) noexcept : TBaseStorage()
    {
      this->insert(this->cend(), std::move(values));
    }

    constexpr InlineVector(size_type count) noexcept : TBaseStorage()
    {
      this->insert(this->cend(), count);
    }

    constexpr InlineVector(size_type count, const value_type &value) noexcept : TBaseStorage()
    {
      this->insert(this->cend(), count, value);
    }

    template <typename TFirst, typename TLast>
    requires(!Integral<TFirst> && !SameType<remove_cvref_t<TLast>, value_type>)
    constexpr InlineVector(TFirst first, TLast last) noexcept : TBaseStorage()
    {
      this->insert(this->cend(), std::move(first), std::move(last));
    }

    template <typename... TArgs>
    constexpr reference emplace_back(TArgs &&...args) noexcept(NoThrowConstructible<value_type, TArgs...>)
    {
      auto *targetPtr = this->Data(this->_layout.Size);
      Krys::construct_at(targetPtr, std::forward<TArgs>(args)...);
      ++this->_layout.Size;
      return *targetPtr;
    }

    constexpr reference push_back(const value_type &value) noexcept(NoThrowCopyConstructible<value_type>)
    {
      auto *targetPtr = this->Data(this->_layout.Size);
      Krys::construct_at(targetPtr, value);
      ++this->_layout.Size;
      return *targetPtr;
    }

    constexpr reference push_back(value_type &&value) noexcept(NoThrowMoveConstructible<value_type>)
    {
      auto *targetPtr = this->Data(this->_layout.Size);
      Krys::construct_at(targetPtr, std::move(value));
      ++this->_layout.Size;
      return *targetPtr;
    }

    template <typename... TArgs>
    constexpr reference emplace_front(TArgs &&...args) noexcept(NoThrowConstructible<value_type, TArgs...>)
    {
      return *this->emplace(this->cbegin(), std::forward<TArgs>(args)...);
    }

    constexpr reference push_front(const value_type &value) noexcept(NoThrowCopyConstructible<value_type>)
    {
      return *this->insert(this->cbegin(), value);
    }

    constexpr reference push_front(value_type &&value) noexcept(NoThrowMoveConstructible<value_type>)
    {
      return *this->insert(this->cbegin(), std::move(value));
    }

    template <typename... TArgs>
    constexpr iterator emplace(const_iterator where, TArgs &&...args) noexcept(IsEmplaceNoexcept<TArgs...>())
    {
      assert(inline_capacity > this->size());
      return this->EmplaceNoCapacityCheck(std::move(where), std::forward<TArgs>(args)...);
    }

    constexpr iterator insert(const_iterator where,
                              value_type &&value) noexcept(NoThrowMoveConstructible<value_type>
                                                           && (NoThrowMoveAssignable<value_type>
                                                               || NoThrowCopyAssignable<value_type>))
    {
      return this->emplace(std::move(where), std::move(value));
    }

    constexpr iterator insert(const_iterator where,
                              const value_type &value) noexcept(NoThrowCopyConstructible<value_type>
                                                                && (NoThrowMoveAssignable<value_type>
                                                                    || NoThrowCopyAssignable<value_type>))
    {
      return this->emplace(std::move(where), value);
    }

    constexpr iterator insert(const_iterator where, std::initializer_list<value_type> values) noexcept(
      NoThrowCopyConstructible<value_type>
      && (NoThrowMoveAssignable<value_type> || NoThrowCopyAssignable<value_type>))
    {
      return this->insert(std::move(where), values.begin(), values.end());
    }

    constexpr iterator insert(const_iterator where, size_type count, const value_type &value) noexcept
    {
      if (count == 0)
      {
        return iterator(const_cast<pointer>(where.base()));
      }
      assert(inline_capacity > this->size());
      difference_type whereDist = where - this->begin();
      assert(whereDist < this->size());
      assert(static_cast<size_type>(count + whereDist) <= this->capacity());
      return this->UncheckedMultiInsertCount(std::move(where), whereDist, count, value);
    }

    template <typename TFirst, typename TLast,
              std::enable_if_t<!Integral<TFirst>                               // cf
                               && !SameType<remove_cvref_t<TLast>, value_type> // cf
                               > * = nullptr>
    constexpr iterator insert(const_iterator where, TFirst first, TLast last)
    {
      if (first == last)
      {
        return ToMutableIter(std::move(where), *this);
      }
      if (this->empty())
      {
        // simply push all directly in
        this->UncheckedMultiInsertEmpty(std::move(first), std::move(last));
        return this->begin();
      }

      assert(inline_capacity > this->size());
      difference_type whereDist = where - this->cbegin();
      assert(static_cast<size_type>(whereDist) < this->size());

      // FIXME: lazy dependent-false
      // idiom-check iterators
      if constexpr (Void<value_type>)
      {
        auto flDist = last - first;
        assert(static_cast<size_type>(flDist + whereDist) <= this->capacity());
        return this->UncheckedMultiInsertRandomAccess(std::move(where), whereDist, std::move(first),
                                                      std::move(last));
      }
      else
      {
        iterator whereFirst = this->emplace(std::move(where), *first);
        ++first;
        iterator whereCurrently = whereFirst;
        for (; first != last; ++first)
        {
          ++whereCurrently;
          whereCurrently = this->emplace(std::move(whereCurrently), *first);
        }
        return whereFirst;
      }
    }

    constexpr iterator erase(const_iterator where) noexcept
    {
      difference_type whereDist = where - this->begin();
      assert(whereDist < this->_layout.Size);
      if (whereDist == this->_layout.Size)
      {
        return ToMutableIter(*this, std::move(where));
      }
      if (whereDist == 0)
      {
        pop_front();
        return this->begin();
      }
      return this->UncheckedErase(std::move(where));
    }

    constexpr iterator erase(const_iterator whereFirst, const_iterator whereLast) noexcept
    {
      if (whereFirst == whereLast)
      {
        return ToMutableIter(*this, std::move(whereLast));
      }
      difference_type whereDiff = whereLast - whereFirst;
      assert(whereDiff < this->Size);
      if (whereDiff == this->size())
      {
        this->clear();
        return this->begin();
      }
      return this->UncheckedMultiErase(std::move(whereFirst), std::move(whereLast), whereDiff);
    }

    constexpr void pop_back() noexcept
    {
      assert(this->_layout.Size > 0);
      Krys::destroy_at(std::addressof(this->Data(this->_layout.Size - 1).value));
      --this->_layout.Size;
    }

    constexpr void pop_front() noexcept
    {
      assert(this->_layout.Size != 0);
      this->UncheckedErase(this->begin());
    }

    constexpr reference front() noexcept
    {
      assert(this->_layout.Size > 0);
      return *this->Data(0);
    }

    constexpr const_reference front() const noexcept
    {
      assert(this->_layout.Size > 0);
      return *this->Data(0);
    }

    constexpr reference back() noexcept
    {
      assert(this->_layout.Size > 0);
      return *this->Data(this->Size - 1);
    }

    constexpr const_reference back() const noexcept
    {
      assert(this->_layout.Size > 0);
      return *this->Data(this->_layout.Size - 1);
    }

    constexpr bool empty() const noexcept
    {
      return this->_layout.Size == 0;
    }

    constexpr bool is_empty() const noexcept
    {
      return this->_layout.Size == 0;
    }

    constexpr void clear() noexcept
    {
      this->DestroyAll();
    }

    constexpr size_type size() const noexcept
    {
      return this->_layout.Size;
    }

    constexpr size_type capacity() const noexcept
    {
      return Capacity;
    }

    constexpr size_type max_size() const noexcept
    {
      return Capacity;
    }

    using TBaseStorage::data;

    constexpr iterator begin() noexcept
    {
      return iterator(this->StorageDataAt(0));
    }

    constexpr const_iterator begin() const noexcept
    {
      return const_iterator(this->StorageDataAt(0));
    }

    constexpr const_iterator cbegin() const noexcept
    {
      return const_iterator(this->StorageDataAt(0));
    }

    constexpr iterator end() noexcept
    {
      return iterator(this->StorageDataAt(this->_layout.Size));
    }

    constexpr const_iterator end() const noexcept
    {
      return const_iterator(this->StorageDataAt(this->_layout.Size));
    }

    constexpr const_iterator cend() const noexcept
    {
      return const_iterator(this->StorageDataAt(this->_layout.Size));
    }

    constexpr reverse_iterator rbegin() noexcept
    {
      return std::make_reverse_iterator(this->end());
    }

    constexpr const_reverse_iterator rbegin() const noexcept
    {
      return std::make_reverse_iterator(this->end());
    }

    constexpr const_reverse_iterator crbegin() const noexcept
    {
      return std::make_reverse_iterator(this->cend());
    }

    constexpr reverse_iterator rend() noexcept
    {
      return std::make_reverse_iterator(this->begin());
    }

    constexpr const_reverse_iterator rend() const noexcept
    {
      return std::make_reverse_iterator(this->begin());
    }

    constexpr const_reverse_iterator crend() const noexcept
    {
      return std::make_reverse_iterator(this->cbegin());
    }

    constexpr reference operator[](size_type index)
    {
      return *this->Data(index);
    }

    constexpr const_reference operator[](size_type index) const
    {
      return *this->Data(index);
    }

    constexpr void resize(size_type count)
    {
      if (count == 0)
      {
        this->clear();
        return;
      }

      if (this->size() < count)
      {
        size_type diff = count - this->size();
        this->insert(this->cend(), diff, value_type {});
      }
      else if (this->size() > count)
      {
        size_type diff = this->size() - count;
        this->erase(this->begin() + diff, this->cend());
      }
    }

    constexpr void resize(size_type count, const value_type &value)
    {
      if (count == 0)
      {
        this->clear();
        return;
      }

      if (this->size() < count)
      {
        size_type diff = count - this->size();
        this->insert(this->cend(), diff, value);
      }
      else if (this->size() > count)
      {
        size_type diff = this->size() - count;
        this->erase(this->begin() + diff, this->cend());
      }
    }

  private:
    template <typename... TArgs>
    constexpr iterator EmplaceNoCapacityCheck(const_iterator where,
                                              TArgs &&...args) noexcept(IsEmplaceNoexcept<TArgs...>())
    {
      iterator whereLast = this->end();
      if (this->empty() || where == whereLast)
      {
        reference __element = this->emplace_back(std::forward<TArgs>(args)...);
        return iterator(std::addressof(__element));
      }
      return this->UncheckedEmplace(std::move(where), std::move(whereLast), std::forward<TArgs>(args)...);
    }

    template <typename... TArgs>
    constexpr iterator UncheckedEmplace(const_iterator where, iterator whereLast,
                                        TArgs &&...args) noexcept(IsEmplaceNoexcept<TArgs...>())
    {
      difference_type whereDist = where - this->cbegin();
      assert(static_cast<std::size_t>(whereDist) < this->size());
      iterator whereFirst = ToMutableIter(where, *this);

      // Step 0: create a new object off-to-the-right
      auto whereMiddleLast = std::prev(whereLast);
      Krys::construct_at(Krys::to_address(whereLast), *whereMiddleLast);
      // Step 1: shift everything to the right, if size is large enough to need it
      if (whereMiddleLast != whereFirst)
      {
        KRYS_MAYBE_UNUSED auto destinationLast = std::move_backward(whereFirst, whereMiddleLast, whereLast);
      }
      // Step 2: construct the new value in place
      Krys::construct_at(Krys::to_address(whereFirst), std::forward<TArgs>(args)...);
      ++this->_layout.Size;
      return whereFirst;
    }

    constexpr iterator UncheckedErase(const_pointer whereConstPointer) noexcept
    {
      iterator whereFirst(const_cast<pointer>(whereConstPointer));
      iterator whereLast = this->begin() + this->size();
      for (iterator whereP = whereFirst, fromWhereP = whereFirst + 1; fromWhereP != whereLast;
           ++whereP, (void)++fromWhereP)
      {
        if constexpr (NoThrowMoveAssignable<value_type>)
        {
          *whereP = std::move(*fromWhereP);
        }
        else if constexpr (NoThrowCopyAssignable<value_type>)
        {
          *whereP = *fromWhereP;
        }
        else
        {
          static_assert(NoThrowCopyAssignable<value_type>,
                        "this type is neither nothrow copy-assignable or nothrow move-constructible, making "
                        "it impossible to properly copy in a constexpr constext in a pre-C++20 world.");
        }
      }
      Krys::destroy_at(Krys::to_address(whereLast));
      --this->_layout.Size;
      return whereFirst;
    }

    template <typename TFirst, typename TLast>
    constexpr void UncheckedMultiInsertEmpty(TFirst first, TLast last)
    {
      for (; first != last; ++first)
      {
        Krys::construct_at(this->Data(this->_layout.Size), *first);
        ++this->_layout.Size;
      }
    }

    constexpr iterator UncheckedMultiErase(const_iterator first, const_iterator last,
                                           difference_type whereDiff) noexcept
    {
      iterator whereFirst(const_cast<pointer>(first.base()));
      iterator whereLast(const_cast<pointer>(last.base()));
      iterator whereDiffLast = whereLast - whereDiff;
      if constexpr (NoThrowMoveAssignable<value_type>)
      {
        for (iterator whereP = whereFirst, fromWhereP = whereDiffLast; fromWhereP != whereFirst;
             ++whereP, (void)++fromWhereP)
        {
          *whereP = std::move(*fromWhereP);
        }
      }
      else if (NoThrowCopyAssignable<value_type>)
      {
        for (iterator whereP = whereFirst, fromWhereP = whereDiffLast; fromWhereP != whereFirst;
             ++whereP, (void)++fromWhereP)
        {
          *whereP = *fromWhereP;
        }
      }
      // FIXME: when construct_at and destroy_at are valid on automatic storage
      // duration variables, we can constexpr destruct then constexpr-construct
#if 0
			else if (NoThrowMoveConstructible<value_type) {
			}
			else if (NoThrowCopyConstructible<value_type>) {
			}
#endif
      else
      {
        static_assert(NoThrowCopyAssignable<value_type>,
                      "this type is neither nothrow copy-assignable or nothrow move-constructible, making it "
                      "impossible to properly copy in a constexpr constext in a pre-C++20 world.");
      }
      for (pointer whereP = whereLast - 1;; --whereP)
      {
        Krys::destroy_at(whereP);
        if (whereP == whereDiffLast)
        {
          break;
        }
      }
      --this->_layout.Size;
      return whereFirst;
    }
  };

  template <typename T>
  class InlineVector<T, 0>
  {
  private:
    template <typename, std::size_t>
    friend class InlineBasicString;

  public:
    using value_type = T;
    using reference = std::add_lvalue_reference_t<T>;
    using const_reference = std::add_lvalue_reference_t<const T>;
    using pointer = std::add_pointer_t<T>;
    using const_pointer = std::add_pointer_t<const T>;
    using iterator = Krys::Ranges::WrappedPointer<T>;
    using const_iterator = Krys::Ranges::WrappedPointer<T>;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    inline static constexpr const std::size_t inline_capacity = 0;

    constexpr InlineVector() noexcept
    {
    }

    constexpr InlineVector(std::initializer_list<value_type> values) noexcept
    {
      assert(values.size() < size_type(1) && "there must be no values");
    }

    constexpr InlineVector(size_type count) noexcept
    {
      assert(count < size_type(1) && "there must be no values");
    }

    template <typename... TArgs>
    reference emplace_back(TArgs &&...) noexcept
    {
      assert(false && "there is no capacity in this InlineVector");
      return *static_cast<pointer>(nullptr);
    }

    reference push_back(const value_type &) noexcept
    {
      assert(false && "there is no capacity in this InlineVector");
      return *static_cast<pointer>(nullptr);
    }

    reference push_back(value_type &&) noexcept
    {
      assert(false && "there is no capacity in this InlineVector");
      return *static_cast<pointer>(nullptr);
    }

    template <typename... TArgs>
    reference emplace_front(TArgs &&...) noexcept
    {
      assert(false && "there is no capacity in this InlineVector");
      return *static_cast<pointer>(nullptr);
    }

    reference push_front(const value_type &) noexcept
    {
      assert(false && "there is no capacity in this InlineVector");
      return *static_cast<pointer>(nullptr);
    }

    reference push_front(value_type &&) noexcept
    {
      assert(false && "there is no capacity in this InlineVector");
      return *static_cast<pointer>(nullptr);
    }

    template <typename... TArgs>
    iterator emplace(const_iterator, TArgs &&...) noexcept
    {
      assert(false && "nothing can be emplaced in this InlineVector");
      return iterator(nullptr);
    }

    constexpr iterator insert(const_iterator, value_type &&) noexcept
    {
      assert(false && "nothing can be inserted in this InlineVector");
      return iterator(nullptr);
    }

    constexpr iterator insert(const_iterator, const value_type &) noexcept
    {
      assert(false && "nothing can be inserted in this InlineVector");
      return iterator(nullptr);
    }

    constexpr iterator insert(const_iterator, std::initializer_list<value_type>) noexcept
    {
      assert(false && "nothing can be inserted in this InlineVector");
      return iterator(nullptr);
    }

    constexpr iterator insert(const_iterator, size_type) noexcept
    {
      assert(false && "nothing can be inserted in this InlineVector");
      return iterator(nullptr);
    }

    constexpr iterator insert(const_iterator, size_type, const value_type &) noexcept
    {
      assert(false && "nothing can be inserted in this InlineVector");
      return iterator(nullptr);
    }

    template <typename TFirst, typename TLast>
    requires(!Integral<TFirst> && !SameType<remove_cvref_t<TLast>, value_type>)
    constexpr iterator insert(const_iterator, TFirst, TLast)
    {
      assert(false && "there is no capacity in this InlineVector");
      return iterator(nullptr);
    }

    constexpr iterator erase(const_iterator) noexcept
    {
      assert(false && "there is nothing to erase in this InlineVector");
      return iterator(nullptr);
    }

    constexpr iterator erase(const_iterator, const_iterator) noexcept
    {
      assert(false && "there is nothing to erase in this InlineVector");
      return iterator(nullptr);
    }

    constexpr void pop_back() noexcept
    {
      assert(false && "there is nothing to erase in this InlineVector");
    }

    constexpr void pop_front() noexcept
    {
      assert(false && "there is nothing to erase in this InlineVector");
    }

    constexpr reference front() noexcept
    {
      assert(false && "there is no capacity in this InlineVector");
      return *static_cast<pointer>(nullptr);
    }

    constexpr const_reference front() const noexcept
    {
      assert(false && "there is no capacity in this InlineVector");
      return *static_cast<const_pointer>(nullptr);
    }

    constexpr reference back() noexcept
    {
      assert(false && "there is no capacity in this InlineVector");
      return *static_cast<pointer>(nullptr);
    }

    constexpr const_reference back() const noexcept
    {
      assert(false && "there is no capacity in this InlineVector");
      return *static_cast<const_pointer>(nullptr);
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

    constexpr pointer data() noexcept
    {
      return nullptr;
    }

    constexpr const_pointer data() const noexcept
    {
      return nullptr;
    }

    constexpr iterator begin() noexcept
    {
      return iterator(nullptr);
    }

    constexpr const_iterator begin() const noexcept
    {
      return const_iterator(nullptr);
    }

    constexpr const_iterator cbegin() const noexcept
    {
      return const_iterator(nullptr);
    }

    constexpr iterator end() noexcept
    {
      return iterator(nullptr);
    }

    constexpr const_iterator end() const noexcept
    {
      return const_iterator(nullptr);
    }

    constexpr const_iterator cend() const noexcept
    {
      return const_iterator(nullptr);
    }

    constexpr reference operator[](size_type)
    {
      assert(false && "there is no capacity in this InlineVector");
      return *static_cast<pointer>(nullptr);
    }

    constexpr const_reference operator[](size_type) const
    {
      assert(false && "there is no capacity in this InlineVector");
      return *static_cast<const_pointer>(nullptr);
    }

    constexpr void resize(size_type count) noexcept
    {
      assert(count < size_type(1) && "there is no capacity in this InlineVector to resize to anything but 0");
    }

    constexpr void resize(size_type count, const value_type &) noexcept
    {
      assert(count < size_type(1) && "there is no capacity in this InlineVector to resize to anything but 0");
    }
  };
}