#pragma once

#include "Krystal.Booey/CSS/Values/Primitives/Concepts.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/NumericRaw.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/PrimitiveKeywordList.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/UnevaluatedCalc.hpp"
#include "Krystal.Core/Assert.hpp"
#include "Krystal.Core/Concepts.hpp"
#include "Krystal.Core/Maths/Base.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/TypeTraits.hpp"

namespace krys::boo::css
{
  // `PrimitiveData` is a bespoke implementation of `Variant<Numeric, Keywords...>`
  // optimized for memory use by allowing numeric types with multiple unit representations
  // (e.g. <length>, <angle>, etc.) to utilize multiple indices for a single smaller payload.
  //
  // FIXME: Generalize this concept to support arbitrary types through traits.

  template <NumericRaw>
  struct PrimitiveNumeric;

  template <Numeric, PrimitiveKeyword...>
  struct PrimitiveNumericOrKeyword;

  // MARK: - Concepts

  // Concept for use checking if a `ChildPrimitiveData`'s types are a subset of
  // `ParentPrimitiveData`'s types.
  // FIXME: Currently limited to the case of Parent<NumericA, KeywordB, ...> and Child == NumericA.
  template <typename ChildPrimitiveData, typename ParentPrimitiveData>
  concept SubsumesChildPrimitiveData =
    (!SameType<ChildPrimitiveData, ParentPrimitiveData>)
    && (SameType<typename ChildPrimitiveData::Index,
                 typename ParentPrimitiveData::Index::NumericType::Base::Index>);

  // MARK: - Markable Support

  struct PrimitiveDataEmptyToken
  {
    constexpr bool operator==(const PrimitiveDataEmptyToken &) const noexcept = default;
  };

  template <typename T>
  struct PrimitiveDataMarkableTraits
  {
    KRYS_NODISCARD static bool IsEmptyValue(const T &value) noexcept
    {
      return value.IsEmpty();
    }

    KRYS_NODISCARD static T EmptyValue() noexcept
    {
      return T(PrimitiveDataEmptyToken {});
    }
  };

  // MARK: - Index

  template <Numeric N, PrimitiveKeyword... Ks>
  struct PrimitiveDataIndex
  {
    template <Numeric, PrimitiveKeyword...>
    friend struct PrimitiveDataIndex;

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

    constexpr static Storage IndexStorageForFirstRaw = 0;
    constexpr static Storage IndexStorageForLastRaw = static_cast<Storage>(UnitTraits::Count - 1);
    constexpr static Storage IndexStorageForCalc = static_cast<Storage>(UnitTraits::Count);
    constexpr static Storage IndexStorageForFirstKeyword = static_cast<Storage>(UnitTraits::Count + 1);
    constexpr static Storage IndexStorageForLastKeyword =
      static_cast<Storage>(UnitTraits::Count + Keywords::Count);
    // (... gap ...)
    constexpr static Storage IndexStorageForEmpty = Limits<Storage>::max() - 1;
    constexpr static Storage IndexStorageForMovedFrom = Limits<Storage>::max();

    KRYS_NODISCARD constexpr static Storage IndexStorageForUnit(UnitType unit) noexcept
    {
      return static_cast<Storage>(IndexStorageForFirstRaw + ToUnderlying(unit));
    }

    KRYS_NODISCARD consteval static Storage
      IndexStorageForKeyword(ValidKeywordForList<Keywords> auto keyword) noexcept
    {
      return static_cast<Storage>(IndexStorageForFirstKeyword + Keywords::OffsetForKeyword(keyword));
    }

    static_assert(UnitTraits::Count + Keywords::Count + 2 <= Limits<Storage>::max());

  private:
    Storage _storage;

  public:
    // MARK: Construction

    constexpr PrimitiveDataIndex(const PrimitiveDataIndex<N, Ks...> &) noexcept = default;

    template <typename T>
    requires(Keywords::Count != 0)
            && (requires { requires SameType<T, PrimitiveDataIndex<typename N::Base>>; })
    constexpr PrimitiveDataIndex(const T &other) noexcept : _storage {other._storage}
    {
    }

    template <typename T>
    requires(Keywords::Count != 0)
            && (requires { requires SameType<T, PrimitiveDataIndex<typename N::Base>>; })
    constexpr PrimitiveDataIndex &operator=(const T &other) noexcept
    {
      _storage = other._storage;
      return *this;
    }

    constexpr explicit PrimitiveDataIndex(Storage storage) noexcept : _storage {storage}
    {
    }

    constexpr PrimitiveDataIndex(UnitType unit) noexcept : _storage {IndexStorageForUnit(unit)}
    {
    }

    constexpr PrimitiveDataIndex(const Raw &raw) noexcept : _storage {IndexStorageForUnit(raw.Unit)}
    {
    }

    constexpr PrimitiveDataIndex(const Calc &) noexcept : _storage {IndexStorageForCalc}
    {
    }

    constexpr PrimitiveDataIndex(ValidKeywordForList<Keywords> auto keyword) noexcept
        : _storage {IndexStorageForKeyword(keyword)}
    {
    }

    constexpr PrimitiveDataIndex(PrimitiveDataEmptyToken) noexcept : _storage {IndexStorageForEmpty}
    {
    }

    // MARK: Assignment

    constexpr PrimitiveDataIndex &operator=(const PrimitiveDataIndex<N, Ks...> &) noexcept = default;

    // MARK: Raw Unit

    KRYS_NODISCARD constexpr typename NumericType::Raw::UnitType Unit() const noexcept
    {
      krys_debug_assert(IsRaw());
      return static_cast<UnitType>(_storage);
    }

    // MARK: Keyword

    template <typename F>
    constexpr decltype(auto) VisitKeyword(F &&f) const noexcept
    {
      krys_debug_assert(_storage <= IndexStorageForLastKeyword);
      return Keywords::VisitKeywordAtOffset(_storage - IndexStorageForFirstKeyword, std::forward<F>(f));
    }

    // MARK: Predicates

    KRYS_NODISCARD constexpr bool IsRaw() const noexcept
    {
      return _storage >= IndexStorageForFirstRaw && _storage <= IndexStorageForLastRaw;
    }

    KRYS_NODISCARD constexpr bool IsCalc() const noexcept
    {
      return _storage == IndexStorageForCalc;
    }

    KRYS_NODISCARD constexpr bool IsKeyword(ValidKeywordForList<Keywords> auto keyword) const noexcept
    {
      return _storage == IndexStorageForKeyword(keyword);
    }

    KRYS_NODISCARD constexpr bool IsEmpty() const noexcept
    {
      return _storage == IndexStorageForEmpty;
    }

    KRYS_NODISCARD constexpr bool IsMovedFrom() const noexcept
    {
      return _storage == IndexStorageForMovedFrom;
    }

    constexpr void SetAsMovedFrom() noexcept
    {
      _storage = IndexStorageForMovedFrom;
    }

    KRYS_NODISCARD constexpr bool operator==(const PrimitiveDataIndex &) const noexcept = default;

    KRYS_NODISCARD constexpr bool operator==(Storage other) const noexcept
    {
      return _storage == other;
    }
  };

  // MARK: - Payload

  union PrimitiveDataPayload
  {
    double Number;
    calc::Value *Calc;

    constexpr PrimitiveDataPayload(double number) noexcept : Number {number}
    {
    }

    constexpr PrimitiveDataPayload(calc::Value *calc) noexcept : Calc {calc}
    {
    }
  };

  // MARK: - PrimitiveData

  template <Numeric N, PrimitiveKeyword... Ks>
  struct PrimitiveData
  {
    template <Numeric, PrimitiveKeyword...>
    friend struct PrimitiveData;

    template <NumericRaw>
    friend struct PrimitiveNumeric;

    template <Numeric, PrimitiveKeyword...>
    friend struct PrimitiveNumericOrKeyword;

    using Index = PrimitiveDataIndex<N, Ks...>;
    using Payload = PrimitiveDataPayload;

    using Keywords = typename Index::Keywords;
    using Raw = typename N::Raw;
    using Calc = typename N::Calc;
    using UnitType = typename N::UnitType;
    using UnitTraits = typename N::UnitTraits;

  private:
    Payload _payload;
    Index _index;

  public:
    PrimitiveData(Raw raw) noexcept : _payload {raw.Value}, _index {raw}
    {
    }

    PrimitiveData(Calc calc) noexcept : _payload {calc.ProtectedCalc().release()}, _index {calc}
    {
    }

    PrimitiveData(ValidKeywordForList<Keywords> auto keyword) noexcept : _payload {0.0}, _index {keyword}
    {
    }

    PrimitiveData(PrimitiveDataEmptyToken token) noexcept : _payload {0.0}, _index {token}
    {
    }

    PrimitiveData(const PrimitiveData &other) noexcept : _payload {other._payload}, _index {other._index}
    {
      if (IsCalc())
      {
        UnevaluatedCalcRef(_payload.Calc);
      }
    }

    PrimitiveData(PrimitiveData &&other) noexcept : _payload {other._payload}, _index {other._index}
    {
      other.SetAsMovedFrom();
    }

    PrimitiveData &operator=(const PrimitiveData &other) noexcept
    {
      // Take the new reference first, so self-assignment can't free the value.
      if (other.IsCalc())
      {
        UnevaluatedCalcRef(other._payload.Calc);
      }

      if (IsCalc())
      {
        UnevaluatedCalcDeref(_payload.Calc);
      }

      _index = other._index;
      _payload = other._payload;

      return *this;
    }

    PrimitiveData &operator=(PrimitiveData &&other) noexcept
    {
      if (this == &other)
      {
        return *this;
      }

      if (IsCalc())
      {
        UnevaluatedCalcDeref(_payload.Calc);
      }

      _index = other._index;
      _payload = other._payload;

      other.SetAsMovedFrom();

      return *this;
    }

    // MARK: Constructor/Assignment for NumericType-only PrimitiveData
    // Allows PrimitiveNumeric<T> to be efficiently assigned to PrimitiveNumericOrKeyword<T, Ks...>.

    template <SubsumesChildPrimitiveData<PrimitiveData> T>
    PrimitiveData(const T &other) noexcept : _payload {other._payload}, _index {other._index}
    {
      if (other.IsCalc())
      {
        UnevaluatedCalcRef(other._payload.Calc);
      }
    }

    template <SubsumesChildPrimitiveData<PrimitiveData> T>
    PrimitiveData(T &&other) noexcept : _payload {other._payload}, _index {other._index}
    {
      other.SetAsMovedFrom();
    }

    template <SubsumesChildPrimitiveData<PrimitiveData> T>
    PrimitiveData &operator=(const T &other) noexcept
    {
      if (other.IsCalc())
      {
        UnevaluatedCalcRef(other._payload.Calc);
      }

      if (IsCalc())
      {
        UnevaluatedCalcDeref(_payload.Calc);
      }

      _index = other._index;
      _payload = other._payload;

      return *this;
    }

    template <SubsumesChildPrimitiveData<PrimitiveData> T>
    PrimitiveData &operator=(T &&other) noexcept
    {
      if (IsCalc())
      {
        UnevaluatedCalcDeref(_payload.Calc);
      }

      _index = other._index;
      _payload = other._payload;

      other.SetAsMovedFrom();

      return *this;
    }

    ~PrimitiveData() noexcept
    {
      if (IsCalc())
      {
        UnevaluatedCalcDeref(_payload.Calc);
      }
    }

    KRYS_NODISCARD bool operator==(const PrimitiveData &other) const noexcept
    {
      if (_index != other._index)
      {
        return false;
      }

      if (IsCalc())
      {
        return AsCalc() == other.AsCalc();
      }

      return _payload.Number == other._payload.Number;
    }

    KRYS_NODISCARD bool operator==(ValidKeywordForList<Keywords> auto other) const noexcept
    {
      return _index == Index(other);
    }

    KRYS_NODISCARD bool operator==(const Raw &raw) const noexcept
    {
      if (_index != Index(raw))
      {
        return false;
      }

      krys_debug_assert(IsRaw());
      return _payload.Number == raw.Value;
    }

    KRYS_NODISCARD bool operator==(const Calc &calc) const noexcept
    {
      if (!IsCalc())
      {
        return false;
      }

      return AsCalc() == calc;
    }

    template <typename T>
    requires NumericRaw<T> && NestedUnitEnumOf<typename T::UnitType, UnitType>
    KRYS_NODISCARD constexpr bool operator==(const T &raw) const noexcept
    {
      if (_index != Index(UnitUpcast<UnitType>(raw.Unit)))
      {
        return false;
      }

      krys_debug_assert(IsRaw());
      return _payload.Number == raw.Value;
    }

    template <UnitType UnitValue>
    KRYS_NODISCARD bool operator==(const ValueLiteral<UnitValue> &literal) const noexcept
    {
      if (_index != Index(literal.Unit))
      {
        return false;
      }

      krys_debug_assert(IsRaw());
      return _payload.Number == literal.Value;
    }

    template <NestedUnitEnumOf<UnitType> E, E UnitValue>
    KRYS_NODISCARD bool operator==(const ValueLiteral<UnitValue> &literal) const noexcept
    {
      if (_index != Index(UnitUpcast<UnitType>(literal.Unit)))
      {
        return false;
      }

      krys_debug_assert(IsRaw());
      return _payload.Number == literal.Value;
    }

    // MARK: Conditional Accessors

    KRYS_NODISCARD Maybe<Raw> GetRaw() const noexcept
    {
      if (IsRaw())
      {
        return AsRaw();
      }

      return null;
    }

    KRYS_NODISCARD Maybe<Calc> GetCalc() const noexcept
    {
      if (IsCalc())
      {
        return AsCalc();
      }

      return null;
    }

    // MARK: Accessors

    KRYS_NODISCARD Raw AsRaw() const noexcept
    {
      krys_debug_assert(IsRaw());
      return Raw {_index.Unit(), _payload.Number};
    }

    KRYS_NODISCARD Calc AsCalc() const noexcept
    {
      krys_debug_assert(IsCalc());
      return Calc {*_payload.Calc};
    }

    KRYS_NODISCARD constexpr bool IsRaw() const noexcept
    {
      return _index.IsRaw();
    }

    KRYS_NODISCARD constexpr bool IsCalc() const noexcept
    {
      return _index.IsCalc();
    }

    KRYS_NODISCARD constexpr bool IsKeyword(ValidKeywordForList<Keywords> auto keyword) const noexcept
    {
      return _index.IsKeyword(keyword);
    }

    KRYS_NODISCARD constexpr bool IsEmpty() const noexcept
    {
      return _index.IsEmpty();
    }

    KRYS_NODISCARD constexpr bool IsMovedFrom() const noexcept
    {
      return _index.IsMovedFrom();
    }

    template <typename T>
    KRYS_NODISCARD bool HoldsAlternative() const noexcept
    {
      if constexpr (SameType<T, Calc>)
      {
        return _index.IsCalc();
      }
      else if constexpr (SameType<T, Raw>)
      {
        return _index.IsRaw();
      }
      else if constexpr (ValidKeywordForList<T, Keywords>)
      {
        return _index.IsKeyword(T {});
      }
    }

    template <typename F>
    decltype(auto) Visit(F &&f) const noexcept
    {
      if (IsRaw())
      {
        return f(AsRaw());
      }

      if (IsCalc())
      {
        return f(AsCalc());
      }

      return _index.VisitKeyword(std::forward<F>(f));
    }

    void SetAsMovedFrom() noexcept
    {
      _index.SetAsMovedFrom();
      _payload.Number = 0;
    }
  };
}
