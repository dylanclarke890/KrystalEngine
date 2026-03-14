#pragma once

#include "Krystal.Lib/Core/Concepts.hpp"
#include "Krystal.Lib/Core/Endian.hpp"
#include "Krystal.Lib/Core/TypeTraits.hpp"
#include "Krystal.Lib/Pointers/ReferenceWrapper.hpp"
#include "Krystal.Lib/Ranges/Algorithm.hpp"
#include "Krystal.Lib/Ranges/Concepts.hpp"
#include "Krystal.Lib/Ranges/Reconstruct.hpp"
#include "Krystal.Lib/Utils/EBCO.hpp"
#include "Krystal.Lib/Utils/ipow.hpp"
#include "Krystal.Lib/Utils/ToUnderlying.hpp"
#include <climits>
#include <cstddef>
#include <iostream>
#include <limits>
#include <memory>
#include <optional>

namespace Krys::Ranges
{
  namespace Impl
  {
    template <typename TWord, typename TRange, bool>
    class WordIteratorStorage : private EBCO<TRange>
    {
    private:
      using TBase = EBCO<TRange>;

    public:
      constexpr WordIteratorStorage() noexcept(NoThrowDefaultConstructible<TRange>) : TBase()
      {
      }

      constexpr WordIteratorStorage(TRange &&range) noexcept(NoThrowMoveConstructible<TRange>)
          : TBase(std::move(range))
      {
      }

      constexpr WordIteratorStorage(const TRange &range) noexcept(NoThrowCopyConstructible<TRange>)
          : TBase(range)
      {
      }

      using TBase::GetValue;
    };

    template <typename TWord, typename TRange>
    class WordIteratorStorage<TWord, TRange, true> : private EBCO<TRange>
    {
    private:
      using TBase = EBCO<TRange>;

    public:
      std::optional<TWord> _value;

      constexpr WordIteratorStorage() noexcept(NoThrowDefaultConstructible<TRange>) : TBase()
      {
      }
      constexpr WordIteratorStorage(TRange &&range) noexcept(NoThrowMoveConstructible<TRange>
                                                             && NoThrowDefaultConstructible<TWord>)
          : TBase(std::move(range)), _value(std::nullopt)
      {
      }
      constexpr WordIteratorStorage(const TRange &range) noexcept(NoThrowCopyConstructible<TRange>
                                                                  && NoThrowDefaultConstructible<TWord>)
          : TBase(range), _value(std::nullopt)
      {
      }

      using TBase::GetValue;
    };

  }

  ///@brief The sentinel type to be paired with a ztd::ranges::WordIterator
  using WordSentinel = std::default_sentinel_t;

  ///@brief An iterator that composes words out of the bits of a provided underlying stored range.
  template <typename TWord, typename TRange, Endian::Type TEndian>
  class WordIterator : private Impl::WordIteratorStorage<
                         TWord, range_reconstruct_t<remove_cvref_t<TRange>>,
                         InputOrOutputRangeExact<range_reconstruct_t<remove_cvref_t<TRange>>>>
  {
  private:
    using TURange = range_reconstruct_t<remove_cvref_t<TRange>>;
    using TBaseIterator = ::std::ranges::iterator_t<TURange>;
    using TBaseSentinel = std::ranges::sentinel_t<TURange>;
    using TBaseReference = std::iter_reference_t<TBaseIterator>;
    using TMaybeVoidBaseValueType = iterator_value_type_t<TBaseIterator>;
    using TBaseValueType = conditional_t<Void<TMaybeVoidBaseValueType>                       // cf
                                           || (!Number<TMaybeVoidBaseValueType>              // cf
                                               && !SameType<TMaybeVoidBaseValueType, byte>), // cf
                                         uchar, TMaybeVoidBaseValueType>;
    using TDifference = iterator_difference_type_t<TBaseIterator>;
    using TSize = iterator_size_type_t<TBaseIterator>;
    using TValue = TWord;
    using TUnsignedValue = make_unsigned_t<TValue>;
    inline constexpr static bool IsInputOrOutput = InputOrOutputRangeExact<TURange>;
    using TBaseStorage = Impl::WordIteratorStorage<TWord, TURange, IsInputOrOutput>;

    static_assert(
      sizeof(TValue) >= sizeof(TBaseValueType),
      "the 'byte' type selected for the WordIterator must not be larger than the value_type of the "
      "iterator that it is meant to view");
    static_assert((sizeof(TValue) % sizeof(TBaseValueType)) == 0,
                  "the 'byte' type selected for the WordIterator must be evenly divisible by the "
                  "iterator that it is meant to view");

    static inline constexpr TSize BaseValuesPerWord = sizeof(TValue) / sizeof(TBaseValueType);

    class WordReference
    {
    private:
      using TUnderlyingBaseValueType = decltype(Krys::AnyEnumOrCharToUnderlying(TBaseValueType {}));
      using TUnderlyingWordType = decltype(Krys::AnyEnumOrCharToUnderlyingUnsigned(std::declval<TWord>()));
      inline static constexpr TUnderlyingWordType baseBitsPerElement =
        static_cast<TUnderlyingWordType>(sizeof(TUnderlyingBaseValueType) * CHAR_BIT);
      inline static constexpr TUnderlyingWordType baseLowestBitMask =
        static_cast<TUnderlyingWordType>(Krys::ipow(2, baseBitsPerElement) - 1);

    public:
      constexpr WordReference(TURange &range) noexcept : _baseRangeRef(range)
      {
      }
      constexpr WordReference(const TURange &range) noexcept : _baseRangeRef(const_cast<TURange &>(range))
      {
      }

      template <typename TValue>
      requires(!SameType<TValue, WordReference>)
      constexpr WordReference &operator=(TValue maybeVal) noexcept
      {
        if constexpr (TEndian == Endian::System
                      && (Endian::System != Endian::Big && Endian::System != Endian::Little))
        {
          static_assert(DependentFalse<TEndian>,
                        "read value from byte stream to native endianness that is neither little nor big "
                        "(byte order is impossible to infer from the standard)");
        }
        static_assert(sizeof(TValue) <= (sizeof(TBaseValueType) * BaseValuesPerWord),
                      "the size of the value type must be less than or equal to the array size");
        TValue val = static_cast<TValue>(maybeVal);
        TBaseValueType writeStorage[BaseValuesPerWord] {};
        auto writeStorageFirst = writeStorage + 0;
        auto writeStorageLast = writeStorage + BaseValuesPerWord;
        if (!std::is_constant_evaluated())
        {
          // just memcpy the data
          std::memcpy(writeStorage, std::addressof(val), sizeof(value_type));
        }
        else
        {
          // God's given, handwritten, bit-splittin'
          // one-way """memcpy""". 😵
          TUnderlyingWordType bitValue = Krys::AnyEnumOrCharToUnderlying(static_cast<value_type>(val));
          auto writeStorageIt = writeStorage + 0;
          for (std::size_t index = 0; index < BaseValuesPerWord; ++index)
          {
            TUnderlyingWordType bitPosition = static_cast<TUnderlyingWordType>(index * baseBitsPerElement);
            TUnderlyingBaseValueType shiftedBitValue =
              static_cast<TUnderlyingBaseValueType>(bitValue >> bitPosition);
            *writeStorageIt = static_cast<TBaseValueType>(shiftedBitValue & baseLowestBitMask);
            ++writeStorageIt;
          }
        }
        if constexpr (TEndian != Endian::System)
        {
          if constexpr (TEndian == Endian::Big)
          {
            ::std::ranges::reverse(writeStorageFirst, writeStorageLast);
          }
          else
          {
            // What about middle endian or some such??
            // No way to detect in "constexpr" properly: just cry.
            static_assert(DependentFalse<TValue>);
          }
        }
        auto &baseRange = this->BaseRange();
        if constexpr (IsInputOrOutput)
        {
          auto result =
            Impl::Copy(writeStorageFirst, writeStorageLast, ::std::ranges::begin(std::move(baseRange)),
                       ::std::ranges::end(std::move(baseRange)));
          this->BaseRange() = Krys::Ranges::reconstruct(std::in_place_type<TURange>, std::move(result.out));
        }
        else
        {
          Impl::Copy(writeStorageFirst, writeStorageLast, ::std::ranges::begin(baseRange),
                     ::std::ranges::end(baseRange));
        }
        return *this;
      }

      constexpr TValue value() const noexcept
      {
        if constexpr (TEndian == Endian::System
                      && (Endian::System != Endian::Big && Endian::System != Endian::Little))
        {
          static_assert(DependentFalse<TValue>,
                        "read value from byte stream to native endianness that is neither little nor big "
                        "(byte order is impossible to infer from the standard)");
        }
        TBaseValueType readStorage[BaseValuesPerWord] {};
        TBaseValueType *readStorageFirst = readStorage + 0;
        std::size_t readStorageSize = ::std::ranges::size(readStorage);
        TUnsignedValue val {};
        if constexpr (IsInputOrOutput)
        {
          // input iterator here (output iterstors cannot be used)
          // to do this kind of work
          // use iterator directly, re-update it when we are done
          // to prevent failure
          auto &baseRange = this->BaseRange();
          auto result =
            Impl::CopyNUnsafe(::std::ranges::begin(std::move(baseRange)), readStorageSize, readStorageFirst);
          this->BaseRange() = Krys::Ranges::reconstruct(
            std::in_place_type<TURange>, std::move(result.in).begin().base(), std::move(baseRange).end());
        }
        else
        {
          // prevent feed-updating iterator through usage here
          // just copy-and-use
          auto baseItCopy = ::std::ranges::begin(this->BaseRange());
          KRYS_MAYBE_UNUSED auto result =
            Impl::CopyNUnsafe(std::move(baseItCopy), readStorageSize, readStorageFirst);
        }
        if constexpr (TEndian == Endian::Big)
        {
          if constexpr ((sizeof(value_type) * CHAR_BIT) > 8)
          {
            TBaseValueType *readStorageLast = readStorage + BaseValuesPerWord;
            ::std::ranges::reverse(readStorageFirst, readStorageLast);
          }
        }
        if (!std::is_constant_evaluated())
        {
          std::size_t readMemoryStorageSize = readStorageSize * sizeof(TBaseValueType);
          std::memcpy(std::addressof(val), readStorageFirst, readMemoryStorageSize);
        }
        else
        {
          // God's given, handwritten, bit-fusin' one-way """memcpy""". 😵
          for (std::size_t index = 0; index < BaseValuesPerWord; ++index)
          {
            TUnderlyingWordType bitValue =
              static_cast<TUnderlyingWordType>(Krys::AnyEnumOrCharToUnderlyingUnsigned(readStorage[index]));
            TUnderlyingWordType bitPosition = static_cast<TUnderlyingWordType>(index * baseBitsPerElement);
            TUnderlyingWordType shiftedBitValue = (bitValue << bitPosition);
            val |= shiftedBitValue;
          }
        }
        return static_cast<TValue>(val);
      }

      constexpr operator TValue() const noexcept
      {
        return this->value();
      }

    private:
      constexpr TURange &BaseRange() const noexcept
      {
        return this->_baseRangeRef.get();
      }

      ReferenceWrapper<TURange> _baseRangeRef;
    };

  public:
    ///@brief The underlying range type.
    using range_type = TURange;

    ///@brief The underlying iterator type.
    using iterator = TBaseIterator;

    ///@brief The underlying sentinel type.
    using sentinel = TBaseSentinel;

    ///@brief The advertised iterator concept.
    using iterator_category =
      std::conditional_t<Krys::Ranges::IsConceptOrBetter<std::random_access_iterator_tag,
                                                         Krys::Ranges::iterator_category_t<TBaseIterator>>,
                         std::random_access_iterator_tag, Krys::Ranges::iterator_category_t<TBaseIterator>>;

    ///@brief The advertised iterator category.
    using iterator_concept =
      std::conditional_t<Krys::Ranges::IsConceptOrBetter<std::random_access_iterator_tag,
                                                         Krys::Ranges::iterator_concept_t<TBaseIterator>>,
                         std::random_access_iterator_tag, Krys::Ranges::iterator_concept_t<TBaseIterator>>;

    ///@brief The difference_type for iterator distances.
    using difference_type = TDifference;

    ///@brief The value_type.
    using value_type = TValue;

    ///@brief The non-const-qualified reference type.
    using reference = std::conditional_t<IsInputOrOutput, value_type &, WordReference>;

    ///@brief The const-qualified reference type.
    using const_reference = std::conditional_t<IsInputOrOutput, const value_type &, WordReference>;

  private:
    static constexpr bool IsDerefNoexcept() noexcept
    {
      if constexpr (IsInputOrOutput)
      {
        return true;
      }
      else
      {
        return noexcept(reference(std::declval<range_type &>()));
      }
    }

    static constexpr bool IsConstDerefNoexcept() noexcept
    {
      if constexpr (IsInputOrOutput)
      {
        return true;
      }
      else
      {
        return noexcept(const_reference(std::declval<range_type &>()));
      }
    }

    static constexpr bool IsCopyNoexcept() noexcept
    {
      return NoThrowCopyConstructible<iterator>;
    }

    static constexpr bool IsRecedeNoexcept() noexcept
    {
      return noexcept(--std::declval<iterator &>());
    }

    static constexpr bool IsAdvanceNoexcept() noexcept
    {
      return noexcept(++std::declval<iterator &>());
    }

  public:
    ///@brief Default default constructor.
    constexpr WordIterator() = default;

    /// @brief Creates a WordIterator that will walk over the specified rage values.
    /// @param[in] baseRange The range to use for iteration.
    constexpr WordIterator(const range_type &baseRange) noexcept(
      NoThrowConstructible<TBaseStorage, const range_type &>)
        : TBaseStorage(baseRange)
    {
    }

    /// @brief Creates a WordIterator that will walk over the specified rage values.
    /// @param[in] baseRange The range to use for iteration.
    constexpr WordIterator(range_type &&baseRange) noexcept(NoThrowConstructible<TBaseStorage, range_type &&>)
        : TBaseStorage(std::move(baseRange))
    {
    }

    /// @brief Creates a WordIterator that will walk over the specified rage values.
    /// @param[in] args The arguments to create the range.
    template <typename TArg, typename... TArgs>
    constexpr WordIterator(std::in_place_t, TArg &&arg,
                           TArgs &&...args) noexcept(NoThrowConstructible<TBaseStorage, TArg, TArgs...>)
        : TBaseStorage(std::forward<TArg>(arg), std::forward<TArgs>(args)...)
    {
    }

    ///@brief Default copy constructor.
    WordIterator(const WordIterator &) = default;

    ///@brief Default move constructor.
    WordIterator(WordIterator &&) = default;

    ///@brief Default copy assignment.

    WordIterator &operator=(const WordIterator &) = default;
    ///@brief Default move assignment.
    WordIterator &operator=(WordIterator &&) = default;

    ///@brief Retrieves the underlying range.
    constexpr range_type range() & noexcept(CopyConstructible<range_type>
                                              ? NoThrowCopyConstructible<range_type>
                                              : NoThrowMoveConstructible<range_type>)
    {
      if constexpr (CopyConstructible<range_type>)
      {
        return this->TBaseStorage::GetValue();
      }
      else
      {
        return std::move(this->TBaseStorage::GetValue());
      }
    }

    ///@brief Retrieves the underlying range.
    constexpr range_type range() const & noexcept(NoThrowCopyConstructible<range_type>)
    {
      return this->TBaseStorage::GetValue();
    }

    ///@brief Retrieves the underlying range.
    constexpr range_type range() && noexcept(NoThrowMoveConstructible<range_type>)
    {
      return std::move(this->TBaseStorage::GetValue());
    }

    ///@brief Shifts the iterator over by +1.
    constexpr WordIterator operator++(int) noexcept(IsCopyNoexcept() && IsAdvanceNoexcept())
    {
      auto copy = *this;
      ++(*this);
      return copy;
    }

    ///@brief Shifts the iterator over by +1.
    constexpr WordIterator &operator++() noexcept(IsAdvanceNoexcept())
    {
      if constexpr (IsInputOrOutput)
      {
        // force read on next dereference
        this->TBaseStorage::_value = std::nullopt;
      }
      else
      {
        auto firstIt = ::std::ranges::begin(std::move(this->TBaseStorage::GetValue()));
        auto lastIt = ::std::ranges::end(std::move(this->TBaseStorage::GetValue()));
        Krys::Ranges::iter_advance(firstIt, BaseValuesPerWord);
        this->TBaseStorage::GetValue() =
          Krys::Ranges::reconstruct(std::in_place_type<TURange>, std::move(firstIt), std::move(lastIt));
      }
      return *this;
    }

    ///@brief Shifts an iterator by -1.
    template <typename TStrawman = range_type>
    constexpr std::enable_if_t<IsRangeIteratorConceptOrBetter<std::bidirectional_iterator_tag, TStrawman>,
                               WordIterator>
      operator--(int) const noexcept(IsCopyNoexcept() && IsRecedeNoexcept())
    {
      auto copy = *this;
      --(*this);
      return copy;
    }

    ///@brief Shifts an iterator over by -1.
    template <typename TStrawman = range_type>
    constexpr std::enable_if_t<IsRangeIteratorConceptOrBetter<std::bidirectional_iterator_tag, TStrawman>,
                               WordIterator &>
      operator--() noexcept
    {
      _recede(this->TBaseStorage::GetValue(), BaseValuesPerWord);
      return *this;
    }

    /// @brief Returns an iterator whose positioned is shifted over by `by` .
    /// @param[in] by The amount to shift the iterator's position by.
    template <typename TStrawman = range_type>
    constexpr std::enable_if_t<IsRangeIteratorConceptOrBetter<std::random_access_iterator_tag, TStrawman>,
                               WordIterator>
      operator+(difference_type by) const noexcept(IsCopyNoexcept() && IsAdvanceNoexcept())
    {
      auto copy = *this;
      copy += by;
      return copy;
    }

    /// @brief Shifts the iterator's position over by `by` .
    /// @param[in] by The amount to shift the iterator's position by.
    template <typename TStrawman = range_type>
    constexpr std::enable_if_t<IsRangeIteratorConceptOrBetter<std::random_access_iterator_tag, TStrawman>,
                               WordIterator &>
      operator+=(difference_type by) noexcept(IsAdvanceNoexcept())
    {
      if (by < static_cast<difference_type>(0))
      {
        return this->operator+=(-by);
      }
      auto firstIt = ::std::ranges::begin(std::move(this->TBaseStorage::GetValue()));
      auto lastIt = ::std::ranges::end(std::move(this->TBaseStorage::GetValue()));
      Krys::Ranges::iter_advance(firstIt, BaseValuesPerWord * by);
      this->TBaseStorage::GetValue() =
        Krys::Ranges::reconstruct(std::in_place_type<TURange>, std::move(firstIt), std::move(lastIt));
      return *this;
    }

    /// @brief Computes the distance between two iterators.
    /// @param[in] right The iterator at the right hand side of the subtraction operation.
    template <typename TStrawman = range_type>
    constexpr std::enable_if_t<IsRangeIteratorConceptOrBetter<std::random_access_iterator_tag, TStrawman>,
                               difference_type>
      operator-(const WordIterator &right) const noexcept
    {
      difference_type __dist = this->TBaseStorage::GetValue() - right.TBaseStorage::GetValue();
      return static_cast<difference_type>(__dist * BaseValuesPerWord);
    }

    /// @brief Returns an iterator whose positioned is shifted over by `by` .
    /// @param[in] by The amount to decrement the iterator's position.
    template <typename TStrawman = range_type>
    constexpr std::enable_if_t<IsRangeIteratorConceptOrBetter<std::random_access_iterator_tag, TStrawman>,
                               WordIterator>
      operator-(difference_type by) const noexcept(IsCopyNoexcept() && IsRecedeNoexcept())
    {
      auto copy = *this;
      copy -= by;
      return copy;
    }

    /// @brief Shifts the position of the iterator over by `by` .
    /// @param[in] by The amount to decrement the iterator's position.
    template <typename TStrawman = range_type>
    constexpr std::enable_if_t<IsRangeIteratorConceptOrBetter<std::random_access_iterator_tag, TStrawman>,
                               WordIterator &>
      operator-=(difference_type by) noexcept(IsRecedeNoexcept())
    {
      if (by < static_cast<difference_type>(0))
      {
        return this->operator+=(-by);
      }
      auto firstIt = ::std::ranges::begin(std::move(this->TBaseStorage::GetValue()));
      auto lastIt = ::std::ranges::end(std::move(this->TBaseStorage::GetValue()));
      _recede(firstIt, BaseValuesPerWord * by);
      this->TBaseStorage::GetValue() =
        Krys::Ranges::reconstruct(std::in_place_type<TURange>, std::move(firstIt), std::move(lastIt));
      return *this;
    }

    /// @brief References the value at the offset `index.`
    /// @param[in] index The offset to index into.
    /// @remarks If this is an input range, the referenced value comes from internal storage.
    template <typename TStrawman = range_type>
    constexpr std::enable_if_t<IsRangeIteratorConceptOrBetter<std::random_access_iterator_tag, TStrawman>,
                               reference>
      operator[](difference_type index) noexcept(IsCopyNoexcept() && IsAdvanceNoexcept())
    {
      auto copy = *this;
      copy += index;
      return *copy;
    }

    /// @brief References the value at the offset `index.`
    /// @param[in] index The offset to index into.
    /// @remarks If this is an input range, the referenced value comes from internal storage.
    template <typename TStrawman = range_type>
    constexpr std::enable_if_t<IsRangeIteratorConceptOrBetter<std::random_access_iterator_tag, TStrawman>,
                               const_reference>
      operator[](difference_type index) const noexcept(IsCopyNoexcept() && IsAdvanceNoexcept())
    {
      auto copy = *this;
      copy += index;
      return *copy;
    }

    /// @brief References to the current value.
    /// @remarks If this is an input range, the value comes from internal storage.
    constexpr reference operator*() noexcept(IsDerefNoexcept())
    {
      if constexpr (IsInputOrOutput)
      {
        if (this->TBaseStorage::_value == std::nullopt)
        {
          this->ReadOne();
        }
        return *this->TBaseStorage::_value;
      }
      else
      {
        return reference(this->TBaseStorage::GetValue());
      }
    }

    /// @brief References to the current value.
    /// @remarks If this is an input range, the value comes from internal storage.
    constexpr const_reference operator*() const noexcept(IsConstDerefNoexcept())
    {
      if constexpr (IsInputOrOutput)
      {
        if (this->TBaseStorage::_value == std::nullopt)
        {
          const_cast<WordIterator *>(this)->ReadOne();
        }
        return *this->TBaseStorage::_value;
      }
      else
      {
        return const_reference(this->TBaseStorage::GetValue());
      }
    }

    /// @brief Checks if the iterator has not reached the sentinel (the end of the range).
    /// @param[in] left The iterator to check.
    friend constexpr bool operator==(const WordIterator &left,
                                     const WordSentinel &) noexcept(noexcept(left.BaseIsEmpty()))
    {
      return left.BaseIsEmpty();
    }

    /// @brief Checks if the iterator has not reached the sentinel (the end of the range).
    /// @param[in] left The iterator to check.
    friend constexpr bool operator!=(const WordIterator &left,
                                     const WordSentinel &) noexcept(noexcept(!left.BaseIsEmpty()))
    {
      return !left.BaseIsEmpty();
    }

    /// @brief Checks if the iterator has reached the sentinel (the end of the range).
    /// @param[in] left The iterator to check.
    /// @param[in] sentinel The sentinel.
    friend constexpr bool operator==(const WordSentinel &sentinel,
                                     const WordIterator &left) noexcept(noexcept(left == sentinel))
    {
      return left == sentinel;
    }

    /// @brief Checks if the iterator has not reached the sentinel (the end of the range).
    /// @param[in] left The iterator to check.
    /// @param[in] sentinel The sentinel.
    friend constexpr bool operator!=(const WordSentinel &sentinel,
                                     const WordIterator &left) noexcept(noexcept(left != sentinel))
    {
      return left != sentinel;
    }

  private:
    constexpr void ReadOne() noexcept(IsDerefNoexcept())
    {
      if constexpr (IsInputOrOutput)
      {
        TWord readWord = WordReference(this->TBaseStorage::GetValue());
        this->TBaseStorage::_value = std::optional<TWord>(readWord);
      }
    }

    constexpr bool BaseIsEmpty() const noexcept
    {
      if constexpr (Krys::Ranges::HasEmptyADL<range_type>)
      {
        return ::std::ranges::empty(this->TBaseStorage::GetValue());
      }
      else
      {
        return ::std::ranges::begin(this->TBaseStorage::GetValue())
               == ::std::ranges::end(this->TBaseStorage::GetValue());
      }
    }
  };
}