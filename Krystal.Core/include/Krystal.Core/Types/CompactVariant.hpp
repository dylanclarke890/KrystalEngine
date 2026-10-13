#pragma once

#include "Krystal.Core/Assert.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Concepts.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/Maths/Base.hpp"
#include "Krystal.Core/Types/Variant.hpp"
#include "Krystal.Core/Utils/Move.hpp"
#include "Krystal.Core/TypeTraits.hpp"
#include "Krystal.Core/Visitor.hpp"
#include <limits>
#include <new>
#include <utility>

namespace krys
{
  /// @brief Types that are neither pointers nor 32 bits or smaller can still be stored in a `CompactVariant`
  /// by specialising this with `HasAlternativeRepresentation = true` and the following functions:
  ///
  ///   constexpr static uint64 EncodeFromArguments(...) noexcept;
  ///   constexpr static uint64 Encode(const T &) noexcept;
  ///   constexpr static uint64 Encode(T &&) noexcept;
  ///   constexpr static T Decode(uint64) noexcept;
  template <typename T>
  struct CompactVariantTraits
  {
    constexpr static bool HasAlternativeRepresentation = false;
  };

  template <typename T>
  concept CompactVariantAlternativeSmallEnough = sizeof(T) <= 4;

  template <typename T>
  concept CompactVariantAlternativePointer = std::is_pointer_v<T> || IsSmartPtr<T>;

  template <typename T>
  concept CompactVariantAlternative =
    CompactVariantAlternativePointer<T> || CompactVariantAlternativeSmallEnough<T>
    || CompactVariantTraits<T>::HasAlternativeRepresentation;

  /// @brief Operations on the packed storage of a `CompactVariant`.
  ///
  /// A `CompactVariant` stores data by bit-packing the variant index and data into a `uint64`:
  /// - Bits 63-56: if a pointer/smart pointer, bits 63-56 of the pointer (useful if the architecture
  ///   supports top-byte ignore and information is stored there). Otherwise zeroes.
  /// - Bits 55-48: the variant index (indicates the type of the alternative).
  /// - Bits 47-0: if a pointer/smart pointer, bits 47-0 of the pointer. Otherwise the encoded data.
  template <CompactVariantAlternative... Ts>
  struct CompactVariantOperations
  {
    using StdVariant = Variant<Ts...>;
    using Index = uint8;
    using Storage = uint64;

    constexpr static Storage MovedFromDataValue = Limits<Storage>::max();
    constexpr static Storage IndexShift = 48;
    constexpr static Storage IndexMask = 0xFFULL << IndexShift;
    constexpr static Storage PayloadMask = ~IndexMask;
    constexpr static Storage TopByteShift = 64 - 8;
    constexpr static Storage TopByteMask = 0xFFULL << TopByteShift;

    KRYS_NODISCARD constexpr static Storage EncodedIndex(Index index) noexcept
    {
      return static_cast<Storage>(index) << IndexShift;
    }

    KRYS_NODISCARD constexpr static Index DecodedIndex(Storage value) noexcept
    {
      return static_cast<Index>((value & IndexMask) >> IndexShift);
    }

    template <typename T, typename U>
    KRYS_NODISCARD static Storage EncodedPayload(U &&payload) noexcept
    {
      Storage data = 0;

      if constexpr (CompactVariantTraits<T>::HasAlternativeRepresentation)
      {
        data = CompactVariantTraits<T>::Encode(std::forward<U>(payload));
      }
      else
      {
        static_assert(sizeof(T) <= sizeof(Storage));
        ::new (static_cast<void *>(&data)) T(std::forward<U>(payload));
      }

      return CheckedPayload<T>(data);
    }

    template <typename T, typename... Args>
    KRYS_NODISCARD static Storage EncodedPayloadFromArguments(Args &&...arguments) noexcept
    {
      Storage data = 0;

      if constexpr (CompactVariantTraits<T>::HasAlternativeRepresentation)
      {
        data = CompactVariantTraits<T>::EncodeFromArguments(std::forward<Args>(arguments)...);
      }
      else
      {
        static_assert(sizeof(T) <= sizeof(Storage));
        ::new (static_cast<void *>(&data)) T(std::forward<Args>(arguments)...);
      }

      return CheckedPayload<T>(data);
    }

    template <typename T, typename F>
    static decltype(auto) DecodedPayload(Storage value, F &&f) noexcept
    {
      Storage maskedData = value & PayloadMask;

      if constexpr (CompactVariantTraits<T>::HasAlternativeRepresentation)
      {
        T decodedData = CompactVariantTraits<T>::Decode(maskedData);
        return f(decodedData);
      }
      else
      {
        T &decodedData = *std::launder(reinterpret_cast<T *>(&maskedData));
        return f(decodedData);
      }
    }

    template <typename T, typename F>
    static decltype(auto) DecodedConstPayload(Storage value, F &&f) noexcept
    {
      Storage maskedData = value & PayloadMask;

      if constexpr (CompactVariantTraits<T>::HasAlternativeRepresentation)
      {
        T decodedData = CompactVariantTraits<T>::Decode(maskedData);
        return f(std::as_const(decodedData));
      }
      else
      {
        T &decodedData = *std::launder(reinterpret_cast<T *>(&maskedData));
        return f(std::as_const(decodedData));
      }
    }

    template <typename T, typename U>
    KRYS_NODISCARD static Storage Encode(U &&argument) noexcept
    {
      return EncodedPayload<T>(std::forward<U>(argument)) | EncodedIndex(AlternativeIndexV<T, StdVariant>);
    }

    template <typename T, typename... Args>
    KRYS_NODISCARD static Storage EncodeFromArguments(Args &&...arguments) noexcept
    {
      return EncodedPayloadFromArguments<T>(std::forward<Args>(arguments)...)
             | EncodedIndex(AlternativeIndexV<T, StdVariant>);
    }

    template <typename... F>
    static decltype(auto) PayloadForData(Storage data, F &&...f) noexcept
    {
      auto visitor = CreateVisitor(std::forward<F>(f)...);
      return TypeForIndex<StdVariant>(DecodedIndex(data), [&]<typename T>() -> decltype(auto)
                                      { return DecodedPayload<T>(data, visitor); });
    }

    template <typename... F>
    static decltype(auto) ConstPayloadForData(Storage data, F &&...f) noexcept
    {
      auto visitor = CreateVisitor(std::forward<F>(f)...);
      return TypeForIndex<StdVariant>(DecodedIndex(data), [&]<typename T>() -> decltype(auto)
                                      { return DecodedConstPayload<T>(data, visitor); });
    }

    static void Destruct(Storage data) noexcept
    {
      if (data == MovedFromDataValue)
      {
        return;
      }

      PayloadForData(data,
                     [&]<typename T>(T &value)
                     {
                       if constexpr (!std::is_trivially_destructible_v<T>)
                       {
                         value.~T();
                       }
                     });
    }

    static void Copy(Storage &to, Storage from) noexcept
    {
      if (from == MovedFromDataValue)
      {
        to = from;
        return;
      }

      PayloadForData(from, [&]<typename T>(T &value)
                     { to = EncodedPayload<T>(value) | EncodedIndex(AlternativeIndexV<T, StdVariant>); });
    }

    static void Move(Storage &to, Storage from) noexcept
    {
      if (from == MovedFromDataValue)
      {
        to = from;
        return;
      }

      PayloadForData(
        from, [&]<typename T>(T &value)
        { to = EncodedPayload<T>(krys::move(value)) | EncodedIndex(AlternativeIndexV<T, StdVariant>); });
    }

    template <typename T>
    KRYS_NODISCARD static bool Equal(Storage a, Storage b) noexcept
    {
      Storage maskedA = a & PayloadMask;
      Storage maskedB = b & PayloadMask;

      if constexpr (CompactVariantTraits<T>::HasAlternativeRepresentation)
      {
        return CompactVariantTraits<T>::Decode(maskedA) == CompactVariantTraits<T>::Decode(maskedB);
      }
      else
      {
        return *std::launder(reinterpret_cast<T *>(&maskedA))
               == *std::launder(reinterpret_cast<T *>(&maskedB));
      }
    }

  private:
    template <typename T>
    KRYS_NODISCARD static Storage CheckedPayload(Storage data) noexcept
    {
      // For data other than pointers, ensure the data doesn't overwrite the top byte (i.e. the top byte
      // should be zero). Pointers may store arbitrary data in the top byte, and that's okay due to TBI.
      if constexpr (!CompactVariantAlternativePointer<T>)
      {
        krys_assert(!(data & TopByteMask));
      }

      // Ensure the bits in the index area are zeroes. Sanity check to make sure the data doesn't overwrite
      // the index.
      krys_assert(!(data & IndexMask));
      return data & PayloadMask;
    }
  };

  /// @brief Acts like a `Variant` with the following differences:
  /// - All alternatives must be pointers, smart pointers, be 32 bits or smaller, or specialise
  ///   `CompactVariantTraits`.
  /// - Can only contain 254 or fewer alternatives.
  /// - Has a more limited API, only offering `HoldsAlternative()` for type checking and `SwitchOn()` for
  ///   value access.
  template <CompactVariantAlternative... Ts>
  class CompactVariant
  {
    static_assert(sizeof...(Ts) < 255);

    using StdVariant = Variant<Ts...>;
    using Index = uint8;
    using Storage = uint64;
    using Operations = CompactVariantOperations<Ts...>;

  private:
    // FIXME: Use a smaller data type if values are small enough / empty.
    Storage _data;

  public:
    template <typename U>
    requires ConstructibleFrom<StdVariant, U> && (!SameType<remove_cvref_t<U>, CompactVariant>)
             && (!IsInPlaceType<remove_cvref_t<U>>) && (!IsInPlaceIndex<remove_cvref_t<U>>)
    CompactVariant(U &&value) noexcept
        : _data {Operations::template Encode<typename VariantBestMatch<StdVariant, U>::type>(
            std::forward<U>(value))}
    {
    }

    template <typename T, typename... Args>
    requires ConstructibleFrom<StdVariant, T>
    explicit CompactVariant(std::in_place_type_t<T>, Args &&...args) noexcept
        : _data {Operations::template EncodeFromArguments<T>(std::forward<Args>(args)...)}
    {
    }

    template <size_t I, typename... Args>
    requires ConstructibleFrom<StdVariant, std::variant_alternative_t<I, StdVariant>>
    explicit CompactVariant(std::in_place_index_t<I>, Args &&...args) noexcept
        : _data {Operations::template EncodeFromArguments<std::variant_alternative_t<I, StdVariant>>(
            std::forward<Args>(args)...)}
    {
    }

    CompactVariant(const CompactVariant &other) noexcept
    {
      Operations::Copy(_data, other._data);
    }

    CompactVariant &operator=(const CompactVariant &other) noexcept
    {
      if (_data == other._data)
      {
        return *this;
      }

      Operations::Destruct(_data);
      Operations::Copy(_data, other._data);

      return *this;
    }

    CompactVariant(CompactVariant &&other) noexcept
    {
      Operations::Move(_data, other._data);

      // Set `other` to the "moved from" state.
      other._data = Operations::MovedFromDataValue;
    }

    CompactVariant &operator=(CompactVariant &&other) noexcept
    {
      if (_data == other._data)
      {
        return *this;
      }

      Operations::Destruct(_data);
      Operations::Move(_data, other._data);

      // Set `other` to the "moved from" state.
      other._data = Operations::MovedFromDataValue;

      return *this;
    }

    template <typename U>
    requires ConstructibleFrom<StdVariant, U> && (!SameType<remove_cvref_t<U>, CompactVariant>)
             && (!IsInPlaceType<remove_cvref_t<U>>) && (!IsInPlaceIndex<remove_cvref_t<U>>)
    CompactVariant &operator=(U &&value) noexcept
    {
      Operations::Destruct(_data);
      _data =
        Operations::template Encode<typename VariantBestMatch<StdVariant, U>::type>(std::forward<U>(value));

      return *this;
    }

    ~CompactVariant() noexcept
    {
      Operations::Destruct(_data);
    }

    void swap(CompactVariant &other) noexcept
    {
      std::swap(_data, other._data);
    }

    template <typename T, typename... Args>
    void emplace(Args &&...args) noexcept
    {
      Operations::Destruct(_data);
      _data = Operations::template EncodeFromArguments<T>(std::forward<Args>(args)...);
    }

    template <size_t I, typename... Args>
    void emplace(Args &&...args) noexcept
    {
      Operations::Destruct(_data);
      _data = Operations::template EncodeFromArguments<std::variant_alternative_t<I, StdVariant>>(
        std::forward<Args>(args)...);
    }

    KRYS_NODISCARD constexpr Index index() const noexcept
    {
      return Operations::DecodedIndex(_data);
    }

    KRYS_NODISCARD constexpr bool IsValuelessByMove() const noexcept
    {
      return _data == Operations::MovedFromDataValue;
    }

    template <typename T>
    KRYS_NODISCARD constexpr bool HoldsAlternative() const noexcept
    {
      return index() == AlternativeIndexV<T, StdVariant>;
    }

    template <size_t I>
    KRYS_NODISCARD constexpr bool HoldsAlternative() const noexcept
    {
      static_assert(I < std::variant_size_v<StdVariant>);
      return index() == I;
    }

    template <typename... F>
    decltype(auto) SwitchOn(F &&...f) const noexcept
    {
      return Operations::ConstPayloadForData(_data, std::forward<F>(f)...);
    }

    KRYS_NODISCARD bool operator==(const CompactVariant &other) const noexcept
    {
      if (index() != other.index())
      {
        return false;
      }

      return TypeForIndex<StdVariant>(index(), [&]<typename T>()
                                      { return Operations::template Equal<T>(_data, other._data); });
    }
  };

  /// @brief Utility for making a `CompactVariant` directly from a parameter pack of types.
  template <typename... Ts>
  using CompactVariantWrapper = CompactVariant<Ts...>;
}
