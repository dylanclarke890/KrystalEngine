#pragma once

#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Core/Concepts.hpp"
#include "Krystal.Lib/Core/Tags.hpp"
#include "Krystal.Lib/Core/TypeTraits.hpp"
#include "Krystal.Lib/Mixins/NonCopyMovable.hpp"
#include "Krystal.Lib/Pointers/Concepts.hpp"
#include "Krystal.Lib/Types/Numeric.hpp"
#include <bit>
#include <xhash>

namespace Krys
{
  template <typename T>
  struct DefaultHash
  {
    static_assert(DependentFalse<T>, "DefaultHash is not implemented for this type.");

    /// @brief
    constexpr static bool IsSafeToCompareToEmptyOrDeleted = false;
  };

  template <typename T>
  concept Hashable = requires(const T &t) {
    requires std::equality_comparable<T>;
    { DefaultHash<T>::Hash(t) } -> SameType<size_t>;
  };

  template <typename T>
  concept HashableWithMemberFunction = requires(const T &t) {
    requires std::equality_comparable<T>;
    { t.Hash() } -> SameType<size_t>;
  };

  template <typename T>
  concept StdHashable = requires(const T &t) {
    { std::hash<T> {}(t) } -> SameType<size_t>;
  };

  template <typename T>
  consteval bool IsSafeToCompareToEmptyOrDeleted() noexcept
  {
    if constexpr (requires { T::IsSafeToCompareToEmptyOrDeleted; })
    {
      static_assert(SameType<decltype(T::IsSafeToCompareToEmptyOrDeleted), const bool>);
      return T::IsSafeToCompareToEmptyOrDeleted;
    }
    else
    {
      return false;
    }
  }

  /// @brief FNV-1a 32bit hashing algorithm.
  KRYS_NODISCARD constexpr uint32 fnv1a32(char const *s, size_t count) noexcept
  {
    return ((count ? fnv1a32(s, count - 1) : 2'166'136'261u) ^ s[count]) * 16'777'619u;
  }

  /// @brief FNV-1a 32bit hashing algorithm.
  KRYS_NODISCARD constexpr uint32 fnv1a32(char8 const *s, size_t count) noexcept
  {
    return ((count ? fnv1a32(s, count - 1) : 2'166'136'261u) ^ s[count]) * 16'777'619u;
  }

  template <UnsignedIntegral T>
  KRYS_NODISCARD constexpr size_t UnsignedIntegerHash(T key) noexcept
  {
    if constexpr (OneOf<T, uint8, uint16, uint32>)
    {
      // Thomas Wang's 32 Bit Mix Function: https://burtleburtle.net/bob/hash/integer.html
      uint32 out = key;
      out += ~(out << 15);
      out ^= (out >> 10);
      out += (out << 3);
      out ^= (out >> 6);
      out += ~(out << 11);
      out ^= (out >> 16);
      return out;
    }
    else if constexpr (SameType<T, uint64>)
    {
      // Thomas Wang's 64 bit Mix Function: https://burtleburtle.net/bob/hash/integer.html
      uint64 out = key;
      out += ~(out << 32);
      out ^= (out >> 22);
      out += ~(out << 13);
      out ^= (out >> 8);
      out += (out << 3);
      out ^= (out >> 15);
      out += ~(out << 27);
      out ^= (out >> 31);
      return static_cast<size_t>(out);
    }
    else
    {
      static_assert(false, "Unsupported unsigned integral type for hashing.");
    }
  }

  /// @brief Compound integer hash method
  KRYS_NODISCARD constexpr size_t UnsignedIntegerPairHash(size_t key1, size_t key2) noexcept
  {
    uint32 shortRandom1 = 277'951'225;            // A random 32-bit value.
    uint32 shortRandom2 = 95'187'966;             // A random 32-bit value.
    uint64 longRandom = 19'248'658'165'952'622LL; // A random 64-bit value.

    uint64 product = longRandom * (shortRandom1 * key1 + shortRandom2 * key2);
    uint32 highBits = static_cast<uint32>(product >> (sizeof(uint64) - sizeof(uint32)));
    return highBits;
  }

#pragma region Hash Implementations

  template <Integral T>
  struct IntegerHash
  {
    constexpr static bool IsSafeToCompareToEmptyOrDeleted = true;

    KRYS_NODISCARD constexpr static size_t Hash(T key) noexcept
    {
      return UnsignedIntegerHash(static_cast<sized_unsigned_t<sizeof(T)>>(key));
    }

    KRYS_NODISCARD constexpr static bool Equal(T a, T b) noexcept
    {
      return a == b;
    }
  };

  template <FloatingPoint T>
  struct FloatingPointHash
  {
    using Bits = sized_unsigned_t<sizeof(T)>;

    constexpr static bool IsSafeToCompareToEmptyOrDeleted = true;

    KRYS_NODISCARD constexpr static size_t Hash(T key) noexcept
    {
      return UnsignedIntegerHash(std::bit_cast<Bits>(key));
    }

    KRYS_NODISCARD constexpr static bool Equal(T a, T b) noexcept
    {
      return std::bit_cast<Bits>(a) == std::bit_cast<Bits>(b);
    }
  };

  template <typename T, bool IsSmartPointer>
  struct PtrHashBase;

  template <typename T>
  struct PtrHashBase<T, false /* IsSmartPointer */>
  {
    using type = T;

    constexpr static bool IsSafeToCompareToEmptyOrDeleted = true;

    KRYS_NODISCARD constexpr static size_t Hash(type key) noexcept
    {
      return IntegerHash<uintptr_t>::Hash(reinterpret_cast<uintptr_t>(key));
    }

    KRYS_NODISCARD constexpr static bool Equal(type a, type b) noexcept
    {
      return a == b;
    }
  };

  template <typename T>
  struct PtrHashBase<T, true /* IsSmartPointer */>
  {
    using type = T;
    using PtrType = typename GetPtrHelper<T>::PtrType;
    using UnderlyingType = typename GetPtrHelper<T>::UnderlyingType;

    constexpr static bool IsSafeToCompareToEmptyOrDeleted = true;

    KRYS_NODISCARD constexpr static size_t Hash(add_const_t<UnderlyingType> *key) noexcept
    {
      return IntegerHash<uintptr_t>::Hash(reinterpret_cast<uintptr_t>(key));
    }

    KRYS_NODISCARD constexpr static size_t Hash(const T &key) noexcept
    {
      return Hash(GetPtr(key));
    }

    KRYS_NODISCARD constexpr static bool Equal(add_const_t<UnderlyingType> *a,
                                               add_const_t<UnderlyingType> *b) noexcept
    {
      return a == b;
    }

    KRYS_NODISCARD constexpr static bool Equal(const T &a, const T &b) noexcept
    {
      return GetPtr(a) == GetPtr(b);
    }

    KRYS_NODISCARD constexpr static bool Equal(add_const_t<UnderlyingType> *a, const T &b) noexcept
    {
      return a == GetPtr(b);
    }

    KRYS_NODISCARD constexpr static bool Equal(const T &a, add_const_t<UnderlyingType> *b) noexcept
    {
      return GetPtr(a) == b;
    }
  };

  template <typename T>
  struct PtrHash : public PtrHashBase<T, IsSmartPtr<T>>
  {
  };

  template <typename T, typename U>
  struct PairHash
  {
    constexpr static bool IsSafeToCompareToEmptyOrDeleted =
      DefaultHash<T>::IsSafeToCompareToEmptyOrDeleted && DefaultHash<U>::IsSafeToCompareToEmptyOrDeleted;

    KRYS_NODISCARD constexpr static size_t Hash(const std::pair<T, U> &p) noexcept
    {
      return UnsignedIntegerPairHash(DefaultHash<T>::Hash(p.first), DefaultHash<U>::Hash(p.second));
    }

    KRYS_NODISCARD constexpr static bool Equal(const std::pair<T, U> &a, const std::pair<T, U> &b) noexcept
    {
      return DefaultHash<T>::Equal(a.first, b.first) && DefaultHash<U>::Equal(a.second, b.second);
    }
  };

  template <typename T, typename U>
  struct IntegerPairHash
  {
    constexpr static bool IsSafeToCompareToEmptyOrDeleted = PairHash<T, U>::IsSafeToCompareToEmptyOrDeleted;

    KRYS_NODISCARD constexpr static size_t Hash(const std::pair<T, U> &p) noexcept
    {
      return UnsignedIntegerPairHash(p.first, p.second);
    }

    KRYS_NODISCARD constexpr static bool Equal(const std::pair<T, U> &a, const std::pair<T, U> &b) noexcept
    {
      return PairHash<T, U>::Equal(a, b);
    }
  };

  template <typename... Types>
  struct TupleHash
  {
    constexpr static bool IsSafeToCompareToEmptyOrDeleted =
      (DefaultHash<Types>::IsSafeToCompareToEmptyOrDeleted && ...);

    template <size_t I = 0uz>
    requires(I < sizeof...(Types) - 1uz)
    KRYS_NODISCARD constexpr static size_t Hash(const std::tuple<Types...> &t) noexcept
    {
      using IthElementType = std::tuple_element<I, std::tuple<Types...>>::type;
      return UnsignedIntegerPairHash(DefaultHash<IthElementType>::Hash(std::get<I>(t)), Hash<I + 1uz>(t));
    }

    template <size_t I = 0uz>
    requires(Iuz == sizeof...(Types) - 1uz)
    KRYS_NODISCARD constexpr static size_t Hash(const std::tuple<Types...> &t) noexcept
    {
      using IthElementType = std::tuple_element<I, std::tuple<Types...>>::type;
      return DefaultHash<IthElementType>::Hash(std::get<I>(t));
    }

    template <size_t I = 0uz>
    requires(I < sizeof...(Types) - 1uz)
    KRYS_NODISCARD constexpr static bool Equal(const std::tuple<Types...> &a,
                                               const std::tuple<Types...> &b) noexcept
    {
      using IthElementType = std::tuple_element<I, std::tuple<Types...>>::type;
      return DefaultHash<IthElementType>::Equal(std::get<I>(a), std::get<I>(b)) && Equal<I + 1uz>(a, b);
    }

    template <size_t I = 0>
    requires(I == sizeof...(Types) - 1)
    KRYS_NODISCARD constexpr static bool Equal(const std::tuple<Types...> &a,
                                               const std::tuple<Types...> &b) noexcept
    {
      using IthElementType = std::tuple_element<I, std::tuple<Types...>>::type;
      return DefaultHash<IthElementType>::Equal(std::get<I>(a), std::get<I>(b));
    }
  };

  template <HashableWithMemberFunction T>
  struct MemberBasedHash
  {
    constexpr static bool IsSafeToCompareToEmptyOrDeleted = IsSafeToCompareToEmptyOrDeleted<T>();

    KRYS_NODISCARD constexpr static size_t Hash(const T &key) noexcept
    {
      return key.Hash();
    }

    KRYS_NODISCARD constexpr static bool Equal(const T &a, const T &b) noexcept
    {
      return a == b;
    }
  };

  template <typename T>
  struct StdHashAdapter
  {
    constexpr static bool IsSafeToCompareToEmptyOrDeleted = IsSafeToCompareToEmptyOrDeleted<T>();

    KRYS_NODISCARD constexpr static size_t Hash(const T &key) noexcept
    {
      return std::hash<T> {}(key);
    }

    KRYS_NODISCARD constexpr static bool Equal(const T &a, const T &b) noexcept
    {
      return a == b;
    }
  };

#pragma endregion

#pragma region DefaultHash Specializations

  template <typename P>
  struct DefaultHash<P *> : public PtrHash<P *>
  {
  };

  template <>
  struct DefaultHash<bool> : public IntegerHash<uint8_t>
  {
  };

  template <>
  struct DefaultHash<uint8_t> : public IntegerHash<uint8_t>
  {
  };

  template <>
  struct DefaultHash<short> : public IntegerHash<unsigned>
  {
  };

  template <>
  struct DefaultHash<unsigned short> : public IntegerHash<unsigned>
  {
  };

  template <>
  struct DefaultHash<int> : public IntegerHash<unsigned>
  {
  };

  template <>
  struct DefaultHash<unsigned> : public IntegerHash<unsigned>
  {
  };

  template <>
  struct DefaultHash<long> : public IntegerHash<unsigned long>
  {
  };

  template <>
  struct DefaultHash<unsigned long> : public IntegerHash<unsigned long>
  {
  };

  template <>
  struct DefaultHash<long long> : public IntegerHash<unsigned long long>
  {
  };

  template <>
  struct DefaultHash<unsigned long long> : public IntegerHash<unsigned long long>
  {
  };

  template <>
  struct DefaultHash<float> : public FloatingPointHash<float>
  {
  };

  template <>
  struct DefaultHash<double> : public FloatingPointHash<double>
  {
  };

  template <>
  struct DefaultHash<std::pair<short, short>> : public IntegerPairHash<short, short>
  {
  };

  template <>
  struct DefaultHash<std::pair<short, unsigned short>> : public IntegerPairHash<short, unsigned short>
  {
  };

  template <>
  struct DefaultHash<std::pair<short, int>> : public IntegerPairHash<short, int>
  {
  };

  template <>
  struct DefaultHash<std::pair<short, unsigned>> : public IntegerPairHash<short, unsigned>
  {
  };

  template <>
  struct DefaultHash<std::pair<unsigned short, short>> : public IntegerPairHash<unsigned short, short>
  {
  };

  template <>
  struct DefaultHash<std::pair<unsigned short, unsigned short>>
      : public IntegerPairHash<unsigned short, unsigned short>
  {
  };

  template <>
  struct DefaultHash<std::pair<unsigned short, int>> : public IntegerPairHash<unsigned short, int>
  {
  };

  template <>
  struct DefaultHash<std::pair<unsigned short, unsigned>> : public IntegerPairHash<unsigned short, unsigned>
  {
  };

  template <>
  struct DefaultHash<std::pair<int, short>> : public IntegerPairHash<int, short>
  {
  };

  template <>
  struct DefaultHash<std::pair<int, unsigned short>> : public IntegerPairHash<int, unsigned short>
  {
  };

  template <>
  struct DefaultHash<std::pair<int, int>> : public IntegerPairHash<int, int>
  {
  };

  template <>
  struct DefaultHash<std::pair<int, unsigned>> : public IntegerPairHash<unsigned, unsigned>
  {
  };

  template <>
  struct DefaultHash<std::pair<unsigned, short>> : public IntegerPairHash<unsigned, short>
  {
  };

  template <>
  struct DefaultHash<std::pair<unsigned, unsigned short>> : public IntegerPairHash<unsigned, unsigned short>
  {
  };

  template <>
  struct DefaultHash<std::pair<unsigned, int>> : public IntegerPairHash<unsigned, int>
  {
  };

  template <>
  struct DefaultHash<std::pair<unsigned, unsigned>> : public IntegerPairHash<unsigned, unsigned>
  {
  };

  template <typename T, typename U>
  struct DefaultHash<std::pair<T, U>> : public PairHash<T, U>
  {
  };

  template <typename... Types>
  struct DefaultHash<std::tuple<Types...>> : public TupleHash<Types...>
  {
  };

  template <HashableWithMemberFunction T>
  struct DefaultHash<T> : public MemberBasedHash<T>
  {
  };

  template <StdHashable T>
  struct DefaultHash<T> : public StdHashAdapter<T>
  {
  };

#pragma endregion

#pragma region Hash Traits

  template <typename T, bool IsInteger>
  struct GenericHashTraitsBase
  {
    static_assert(DependentFalse<T>, "GenericHashTraitsBase is not implemented for this type.");
  };

  template <typename T>
  struct GenericHashTraitsBase<T, false>
  {
    // The EmptyValueIsZero flag is used to optimize allocation of empty hash tables with zeroed memory.
    constexpr static bool EmptyValueIsZero = false;

    // The HasIsEmptyValueFunction flag allows the hash table to automatically generate code to check
    // for the empty value when it can be done with the equality operator, but allows custom functions
    // for cases like String that need them.
    constexpr static bool HasIsEmptyValueFunction = false;

    // The starting table size. Can be overridden when we know beforehand that
    // a hash table will have at least N entries.
    constexpr static unsigned MinimumTableSize = 8;
  };

  // Default integer traits disallow both 0 and -1 as keys (max value instead of -1 for unsigned).
  template <typename T>
  struct GenericHashTraitsBase<T, true> : public GenericHashTraitsBase<T, false>
  {
    constexpr static bool EmptyValueIsZero = true;

    constexpr static void ConstructDeletedValue(T &slot) noexcept
    {
      slot = static_cast<T>(-1);
    }

    KRYS_NODISCARD constexpr static bool IsDeletedValue(T value) noexcept
    {
      return value == static_cast<T>(-1);
    }
  };

  template <typename T>
  struct GenericHashTraits : public GenericHashTraitsBase<T, Integral<T>>
  {
    using EmptyValueType = T;

    // Type for return value of functions that do not transfer ownership, such as get.
    using PeekType = T;

    using TakeType = T;

    KRYS_NODISCARD constexpr static T EmptyValue() noexcept
    {
      return T();
    }

    template <typename U, typename V>
    constexpr static void AssignToEmpty(U &emptyValue, V &&value) noexcept
    {
      emptyValue = std::forward<V>(value);
    }

    template <typename Traits>
    constexpr static void ConstructEmptyValue(T &slot) noexcept
    {
      new (NotNullTag {}, std::addressof(slot)) T(Traits::EmptyValue());
    }

    template <typename U>
    KRYS_NODISCARD constexpr static U &&Peek(U &&value) noexcept
    {
      return std::forward<U>(value);
    }

    template <typename U>
    KRYS_NODISCARD constexpr static TakeType Take(U &&value) noexcept
    {
      return std::forward<U>(value);
    }
  };

  template <typename T>
  struct HashTraits : public GenericHashTraits<T>
  {
  };

  template <typename T>
  struct FloatHashTraits : public GenericHashTraits<T>
  {
    KRYS_NODISCARD constexpr static T EmptyValue() noexcept
    {
      return std::numeric_limits<T>::infinity();
    }

    constexpr static void ConstructDeletedValue(T &slot) noexcept
    {
      slot = -std::numeric_limits<T>::infinity();
    }

    KRYS_NODISCARD constexpr static bool IsDeletedValue(T value) noexcept
    {
      return value == -std::numeric_limits<T>::infinity();
    }
  };

  template <typename T>
  struct FloatWithZeroEmptyKeyHashTraits : public GenericHashTraits<T>
  {
    constexpr static bool EmptyValueIsZero = true;

    KRYS_NODISCARD constexpr static T EmptyValue() noexcept
    {
      return static_cast<T>(0);
    }

    constexpr static void ConstructDeletedValue(T &slot) noexcept
    {
      slot = -std::numeric_limits<T>::infinity();
    }

    KRYS_NODISCARD constexpr static bool IsDeletedValue(T value) noexcept
    {
      return value == -std::numeric_limits<T>::infinity();
    }
  };

  template <>
  struct HashTraits<float> : public FloatHashTraits<float>
  {
  };

  template <>
  struct HashTraits<double> : public FloatHashTraits<double>
  {
  };

  // Default unsigned traits disallow both 0 and max as keys -- use these traits to allow zero and disallow
  // max - 1.
  template <typename T>
  struct UnsignedWithZeroKeyHashTraits : public GenericHashTraits<T>
  {
    constexpr static bool EmptyValueIsZero = false;

    KRYS_NODISCARD constexpr static T EmptyValue() noexcept
    {
      return std::numeric_limits<T>::max();
    }

    constexpr static void ConstructDeletedValue(T &slot) noexcept
    {
      slot = std::numeric_limits<T>::max() - 1;
    }

    KRYS_NODISCARD constexpr static bool IsDeletedValue(T value) noexcept
    {
      return value == std::numeric_limits<T>::max() - 1;
    }
  };

  template <typename T>
  struct SignedWithZeroKeyHashTraits : public GenericHashTraits<T>
  {
    constexpr static bool EmptyValueIsZero = false;

    KRYS_NODISCARD constexpr static T EmptyValue() noexcept
    {
      return std::numeric_limits<T>::min();
    }

    constexpr static void ConstructDeletedValue(T &slot) noexcept
    {
      slot = std::numeric_limits<T>::max();
    }

    KRYS_NODISCARD constexpr static bool IsDeletedValue(T value) noexcept
    {
      return value == std::numeric_limits<T>::max();
    }
  };

  // Can be used with strong enums, allows zero as key.
  template <typename T>
  struct StrongEnumHashTraits : GenericHashTraits<T>
  {
    using UnderlyingType = typename std::underlying_type<T>::type;

    constexpr static bool EmptyValueIsZero = false;

    KRYS_NODISCARD constexpr static T EmptyValue() noexcept
    {
      return static_cast<T>(std::numeric_limits<UnderlyingType>::max());
    }

    constexpr static void ConstructDeletedValue(T &slot) noexcept
    {
      slot = static_cast<T>(std::numeric_limits<UnderlyingType>::max() - 1);
    }

    KRYS_NODISCARD constexpr static bool IsDeletedValue(T value) noexcept
    {
      return value == static_cast<T>(std::numeric_limits<UnderlyingType>::max() - 1);
    }
  };

  template <IsEnum T>
  struct HashTraits<T> : public StrongEnumHashTraits<T>
  {
  };

  template <typename P>
  struct HashTraits<P *> : public GenericHashTraits<P *>
  {
    constexpr static bool EmptyValueIsZero = true;

    constexpr static void ConstructDeletedValue(P *&slot) noexcept
    {
      slot = reinterpret_cast<P *>(-1);
    }

    KRYS_NODISCARD constexpr static bool IsDeletedValue(P *value) noexcept
    {
      return value == reinterpret_cast<P *>(-1);
    }
  };

#pragma endregion

  struct Hash
  {
    template <typename... Args>
    KRYS_NODISCARD constexpr static size_t Combine(Args... args) noexcept
    {
      size_t seed = 0u;
      Combine(seed, args...);
      return seed;
    }

  private:
    template <typename Head, typename... Tail>
    constexpr static void Combine(size_t &seed, const Head &head, Tail... tail) noexcept
    {
      // this algorithm is based on boost::combine_hash and is designed to mix/smear around values
      seed ^= DefaultHash<Head>::Hash(head) + 0x9e'37'79'b9'7f'4a'7c'55 + (seed << 6) + (seed >> 2);
      (Combine(seed, tail), ...);
    }
  };
}