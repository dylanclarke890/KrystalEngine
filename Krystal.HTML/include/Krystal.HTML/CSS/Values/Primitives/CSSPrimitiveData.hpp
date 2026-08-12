#pragma once

#include "Krystal.HTML/CSS/Calc/CSSCalcValue.hpp"
#include "Krystal.HTML/CSS/Values/Primitives/CSSPrimitiveKeywordList.hpp"
#include "Krystal.HTML/CSS/Values/Primitives/CSSPrimitiveNumericConcepts.hpp"
#include "Krystal.HTML/CSS/Values/Primitives/CSSPrimitiveNumericRaw.hpp"
#include "Krystal.HTML/CSS/Values/Primitives/CSSUnevaluatedCalc.hpp"
#include "Krystal.Lib/Core/Concepts.hpp"
#include "Krystal.Lib/Pointers/RawPtr.hpp"
#include <limits>
#include <type_traits>

namespace Krys::HTML
{
  // `PrimitiveData` is a bespoke implementation of `Variant<Numeric, Keywords...>`
  // optimized for memory use by allowing numeric types with multiple unit representations
  // (e.g. <length>, <angle>, etc.) to utilize multiple indices for a single smaller payload.
  //
  // FIXME: Generalize this concept to support arbitrary types through traits.

  // MARK: - Concepts

  // Concept for use checking if a `ChildPrimitiveData`'s types are a subset of
  // `ParentPrimitiveData`'s types.
  // FIXME: Currently limited to the case of Parent<NumericA, KeywordB, ...> and Child == NumericA.
  template <typename ChildPrimitiveData, typename ParentPrimitiveData>
  concept SubsumesChildPrimitiveData =
    (!SameType<ChildPrimitiveData, ParentPrimitiveData>)
    && (SameType<typename ChildPrimitiveData::Index,
                 typename ParentPrimitiveData::Index::NumericType::Base::Index>);

  // MARK: - Index

  struct PrimitiveDataEmptyToken
  {
    constexpr bool operator==(const PrimitiveDataEmptyToken &) const = default;
  };

  template <Numeric N, PrimitiveKeyword... Ks>
  struct PrimitiveDataIndex
  {
    using NumericType = N;
    using Keywords = PrimitiveKeywordList<Ks...>;

    using Raw = typename N::Raw;
    using Calc = typename N::Calc;
    using UnitType = typename N::UnitType;
    using UnitTraits = typename N::UnitTraits;
    using Storage = underlying_t<typename N::UnitType>;

    // The potential values for the `index` are:
    //  - 0 ... # of units - 1                              -> Raw
    //  - # of units                                        -> Calc
    //  - # of units + 1 ... # of units + # of keywords     -> Constant<Id>
    //
    // (... gap ...)
    //
    //  - max(index_type) - 1                               -> Empty (for Markable)
    //  - max(index_type)                                   -> Moved from

    constexpr static Storage IndexStorageForFirstRaw = 0uz;
    constexpr static Storage IndexStorageForLastRaw = UnitTraits::Count - 1uz;
    constexpr static Storage IndexStorageForCalc = UnitTraits::Count;
    constexpr static Storage IndexStorageForFirstKeyword = UnitTraits::Count + 1uz;
    constexpr static Storage IndexStorageForLastKeyword = UnitTraits::Count + Keywords::Count;
    // (... gap ...)
    constexpr static Storage IndexStorageForEmpty = std::numeric_limits<Storage>::max() - 1uz;
    constexpr static Storage IndexStorageForMovedFrom = std::numeric_limits<Storage>::max();

    KRYS_NODISCARD static constexpr Storage IndexStorageForUnit(UnitType unit) noexcept
    {
      return IndexStorageForFirstRaw + ToUnderlying(unit);
    }

    KRYS_NODISCARD static consteval Storage
      IndexStorageForKeyword(ValidKeywordForList<Keywords> auto keyword) noexcept
    {
      return IndexStorageForFirstKeyword + Keywords::OffsetForKeyword(keyword);
    }

    static_assert(UnitTraits::Count + Keywords::Count + 2uz <= std::numeric_limits<Storage>::max());

    // MARK: Construction

    PrimitiveDataIndex(const PrimitiveDataIndex<N, Ks...> &) noexcept = default;

    template <typename T>
    requires(Keywords::Count != 0uz)
            && (requires { requires SameType<T, PrimitiveDataIndex<typename N::Base>>; })
    PrimitiveDataIndex(const T &other) : storage {other.storage}
    {
    }

    template <typename T>
    requires(Keywords::Count != 0uz)
            && (requires { requires SameType<T, PrimitiveDataIndex<typename N::Base>>; })
    PrimitiveDataIndex &operator=(const T &other)
    {
      storage = other.storage;
      return *this;
    }

    constexpr explicit PrimitiveDataIndex(Storage storage) : storage {storage}
    {
    }

    constexpr PrimitiveDataIndex(UnitType unit) : storage {IndexStorageForUnit(unit)}
    {
    }

    constexpr PrimitiveDataIndex(const Raw &raw) : storage {IndexStorageForUnit(raw.Unit)}
    {
    }

    constexpr PrimitiveDataIndex(const Calc &) : storage {IndexStorageForCalc}
    {
    }

    constexpr PrimitiveDataIndex(ValidKeywordForList<Keywords> auto keyword)
        : storage {IndexStorageForKeyword(keyword)}
    {
    }

    constexpr PrimitiveDataIndex(PrimitiveDataEmptyToken) : storage {IndexStorageForEmpty}
    {
    }

    // MARK: Assignment

    PrimitiveDataIndex &operator=(const PrimitiveDataIndex<N, Ks...> &) noexcept = default;

    // MARK: Raw Unit

    constexpr typename NumericType::Raw::UnitType Unit() const
    {
      assert(IsRaw());
      return static_cast<UnitType>(storage);
    }

    // MARK: Keyword

    template <typename F>
    constexpr decltype(auto) VisitKeyword(F &&f) const
    {
      assert(storage <= IndexStorageForLastKeyword);
      return Keywords::VisitKeywordAtOffset(storage - IndexStorageForFirstKeyword, std::forward<F>(f));
    }

    // MARK: Predicates

    constexpr bool IsRaw() const
    {
      return storage >= IndexStorageForFirstRaw && storage <= IndexStorageForLastRaw;
    }

    constexpr bool IsCalc() const
    {
      return storage == IndexStorageForCalc;
    }

    constexpr bool IsKeyword(ValidKeywordForList<Keywords> auto keyword) const
    {
      return storage == IndexStorageForKeyword(keyword);
    }

    constexpr bool IsEmpty() const
    {
      return storage == IndexStorageForEmpty;
    }

    constexpr bool IsMovedFrom() const
    {
      return storage == IndexStorageForMovedFrom;
    }

    void SetAsMovedFrom()
    {
      storage = IndexStorageForMovedFrom;
    }

    constexpr bool operator==(const PrimitiveDataIndex &) const = default;
    constexpr bool operator==(Storage other) const
    {
      return storage == other;
    }

    Storage storage;
  };

  // MARK: - Payload

  union PrimitiveDataPayload
  {
    double number;
    RawPtr<CSSCalcValue> calc;

    PrimitiveDataPayload(double number) : number {number}
    {
    }

    PrimitiveDataPayload(RawPtr<CSSCalcValue> calc) : calc {calc}
    {
    }
  };

  // MARK: - PrimitiveData

  template <Numeric N, PrimitiveKeyword... Ks>
  struct PrimitiveData
  {
    using Index = PrimitiveDataIndex<N, Ks...>;
    using Payload = PrimitiveDataPayload;

    using Keywords = typename Index::Keywords;
    using Raw = typename N::Raw;
    using Calc = typename N::Calc;
    using UnitType = typename N::UnitType;
    using UnitTraits = typename N::UnitTraits;

    Payload payload;
    Index index;

    PrimitiveData(Raw raw) : payload {raw.value}, index {raw}
    {
    }

    PrimitiveData(Calc calc) : payload {&calc.protectedCalc().leakRef()}, index {calc}
    {
    }

    PrimitiveData(ValidKeywordForList<Keywords> auto keyword) : payload {0.0}, index {keyword}
    {
    }

    PrimitiveData(PrimitiveDataEmptyToken token) : payload {0.0}, index {token}
    {
    }

    PrimitiveData(const PrimitiveData &other) : payload {other.payload}, index {other.index}
    {
      if (IsCalc())
      {
        UnevaluatedCalcAddRef(payload.calc);
      }
    }

    PrimitiveData(PrimitiveData &&other) : payload {other.payload}, index {other.index}
    {
      other.SetAsMovedFrom();
    }

    PrimitiveData &operator=(const PrimitiveData &other)
    {
      if (IsCalc())
      {
        UnevaluatedCalcSubRef(payload.calc);
      }
      if (other.IsCalc())
      {
        UnevaluatedCalcAddRef(other.payload.calc);
      }

      index = other.index;
      payload = other.payload;

      return *this;
    }

    PrimitiveData &operator=(PrimitiveData &&other)
    {
      if (IsCalc())
      {
        UnevaluatedCalcSubRef(payload.calc);
      }

      index = other.index;
      payload = other.payload;

      other.SetAsMovedFrom();

      return *this;
    }

    // MARK: Constructor/Assignment for NumericType-only PrimitiveData
    // Allows PrimitiveNumeric<T> to be efficiently assigned to PrimitiveNumericOrKeyword<T, Ks...>.

    template <SubsumesChildPrimitiveData<PrimitiveData> T>
    PrimitiveData(const T &other) : payload {other.payload}, index {other.index}
    {
      if (other.IsCalc())
      {
        UnevaluatedCalcAddRef(other.payload.calc);
      }
    }

    template <SubsumesChildPrimitiveData<PrimitiveData> T>
    PrimitiveData(T &&other) : payload {other.payload}, index {other.index}
    {
      other.SetAsMovedFrom();
    }

    template <SubsumesChildPrimitiveData<PrimitiveData> T>
    PrimitiveData &operator=(const T &other)
    {
      if (IsCalc())
      {
        UnevaluatedCalcSubRef(payload.calc);
      }
      if (other.IsCalc())
      {
        UnevaluatedCalcAddRef(other.payload.calc);
      }

      index = other.index;
      payload = other.payload;

      return *this;
    }

    template <SubsumesChildPrimitiveData<PrimitiveData> T>
    PrimitiveData &operator=(T &&other)
    {
      if (IsCalc())
      {
        UnevaluatedCalcSubRef(payload.calc);
      }

      index = other.index;
      payload = other.payload;

      other.SetAsMovedFrom();

      return *this;
    }

    ~PrimitiveData()
    {
      if (IsCalc())
      {
        UnevaluatedCalcSubRef(payload.calc);
      }
    }

    bool operator==(const PrimitiveData &other) const
    {
      if (index != other.index)
      {
        return false;
      }

      if (IsCalc())
      {
        return AsCalc() == other.AsCalc();
      }
      return payload.number == other.payload.number;
    }

    bool operator==(ValidKeywordForList<Keywords> auto other) const
    {
      return index == Index(other);
    }

    bool operator==(const Raw &raw) const
    {
      if (index != Index(raw))
      {
        return false;
      }

      assert(IsRaw());
      return payload.number == raw.value;
    }

    bool operator==(const Calc &calc) const
    {
      if (!IsCalc())
      {
        return false;
      }
      return AsCalc() == calc;
    }

    template <typename T>
    requires NumericRaw<T> && NestedUnitEnumOf<typename T::UnitType, UnitType>
    constexpr bool operator==(const T &raw) const
    {
      if (index != Index(UnitUpcast<UnitType>(raw.unit)))
      {
        return false;
      }

      assert(IsRaw());
      return payload.number == raw.value;
    }

    template <UnitType unitValue>
    bool operator==(const ValueLiteral<unitValue> &literal) const
    {
      if (index != Index(literal.unit))
      {
        return false;
      }

      assert(IsRaw());
      return payload.number == literal.value;
    }

    template <NestedUnitEnumOf<UnitType> E, E unitValue>
    bool operator==(const ValueLiteral<unitValue> &literal) const
    {
      if (index != Index(UnitUpcast<UnitType>(literal.unit)))
      {
        return false;
      }

      assert(IsRaw());
      return payload.number == literal.value;
    }

    // MARK: Conditional Accessors

    Maybe<Raw> raw() const
    {
      if (IsRaw())
      {
        return AsRaw();
      }
      return Null;
    }

    Maybe<Calc> calc() const
    {
      if (IsCalc())
      {
        return AsCalc();
      }
      return Null;
    }

    // MARK: Accessors

    Raw AsRaw() const
    {
      assert(IsRaw());
      return Raw {index.Unit(), payload.number};
    }

    Calc AsCalc() const
    {
      assert(IsCalc());
      return Calc {*payload.calc};
    }

    constexpr bool IsRaw() const
    {
      return index.IsRaw();
    }
    constexpr bool IsCalc() const
    {
      return index.IsCalc();
    }
    constexpr bool IsKeyword(ValidKeywordForList<Keywords> auto keyword) const
    {
      return index.IsKeyword(keyword);
    }
    constexpr bool IsEmpty() const
    {
      return index.IsEmpty();
    }
    constexpr bool IsMovedFrom() const
    {
      return index.IsMovedFrom();
    }

    template <typename T>
    bool HoldsAlternative() const
    {
      if constexpr (SameType<T, Calc>)
      {
        return index.IsCalc();
      }
      else if constexpr (SameType<T, Raw>)
      {
        return index.IsRaw();
      }
      else if constexpr (ValidKeywordForList<T, Keywords>)
      {
        return index.IsKeyword(T {});
      }
    }

    template <typename F>
    decltype(auto) Visit(F &&f) const
    {
      if (IsRaw())
      {
        return f(AsRaw());
      }
      if (IsCalc())
      {
        return f(AsCalc());
      }
      return index.VisitKeyword(std::forward<F>(f));
    }

    void SetAsMovedFrom()
    {
      index.SetAsMovedFrom();
      payload.number = 0;
    }
  };
}