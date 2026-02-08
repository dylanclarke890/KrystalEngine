#pragma once

#include "Krystal.Lib/Core/Concepts.hpp"
#include "Krystal.Lib/Core/TypeTraits.hpp"
#include "Krystal.Lib/Detection/AddressSpaceBitSize.hpp"
#include "Krystal.Lib/Pointers/CompactPtr.hpp"
#include "Krystal.Lib/Types/Numeric.hpp"
#include <bit>
#include <cassert>
#include <utility>

namespace Krys
{
  template <typename T>
  concept CompactablePtr = IsPointer<T> && Krys::AllowsCompactPointers<T>();

  template <typename T>
  concept IntegralOrEnum = Integral<T> || IsEnum<T>;

  /// @brief Folds a pointer and 2 bytes value into 8 bytes in both 32bit and 64bit architectures.
  /// @note We are assuming 48bit pointers for 64bit architectures. 32bit architecture just has a pair of
  /// bytes and a pointer, which should be 8 bytes.
  template <CompactablePtr TPtr, IntegralOrEnum TData>
  class CompactPtrTuple final
  {
    using TUnsigned = make_unsigned_t<conditional_t<SameType<TData, bool>, uint8, TData>>;
    static_assert(sizeof(TData) <= 2 && sizeof(TUnsigned) == sizeof(TData));

    template <CompactablePtr, IntegralOrEnum>
    friend class CompactPtrTuple;

  private:
#if KRYS_ADDRESS_SPACE(64)
    uint64 _data {0u};
#else
    TPtr _ptr {nullptr};
    TData _data {0};
#endif

  public:
    CompactPtrTuple() = default;

    friend bool operator==(const CompactPtrTuple &, const CompactPtrTuple &) = default;

#if KRYS_ADDRESS_SPACE(64)
  public:
    static constexpr uint8 MaxPtrBits = 48;
    static_assert(KRYS_EFFECTIVE_ADDRESS_WIDTH <= MaxPtrBits);

    static constexpr uint64_t PtrMask = (1ULL << MaxPtrBits) - 1;

    CompactPtrTuple(TPtr ptr, TData data) noexcept : _data(Encode(ptr, data))
    {
      assert(this->Data() == data);
      assert(this->Ptr() == ptr);
    }

    template <typename TOther>
    requires(CompactablePtr<TOther> && ConvertibleTo<TOther, TPtr>)
    CompactPtrTuple(CompactPtrTuple<TOther, TData> &&other) noexcept : _data {std::exchange(other._data, {})}
    {
    }

    KRYS_NODISCARD constexpr TPtr Ptr() const noexcept
    {
      return std::bit_cast<TPtr>(_data & PtrMask);
    }

    constexpr void SetPtr(TPtr ptr) noexcept
    {
      _data = Encode(ptr, Data());
      assert(this->Ptr() == ptr);
    }

    KRYS_NODISCARD constexpr TData Data() const noexcept
    {
      return DecodeData(_data);
    }

    constexpr void SetData(TData data) noexcept
    {
      _data = Encode(Ptr(), data);
      assert(this->Data() == data);
    }

    KRYS_NODISCARD constexpr uint64 data() const noexcept
    {
      return _data;
    }

    constexpr void swap(CompactPtrTuple &other) noexcept
    {
      std::swap(_data, other._data);
    }

  private:
    KRYS_NODISCARD constexpr static uint64 EncodeData(TData data) noexcept
    {
      return static_cast<uint64>(static_cast<TUnsigned>(data)) << MaxPtrBits;
    }

    KRYS_NODISCARD constexpr static TData DecodeData(uint64_t value) noexcept
    {
      return static_cast<TData>(static_cast<TUnsigned>(value >> MaxPtrBits));
    }

    KRYS_NODISCARD constexpr static uint64 Encode(TPtr pointer, TData data) noexcept
    {
      return std::bit_cast<uint64>(pointer) | EncodeData(data);
    }
#else
  public:
    constexpr CompactPtrTuple(TPtr ptr, TData TData) noexcept : _ptr(ptr), _data(TData)
    {
    }

    template <IsPointer TOther>
    requires(ConvertibleTo<TOther, TPtr>)
    constexpr CompactPtrTuple(CompactPtrTuple<TOther, TData> &&other) noexcept
        : _ptr {std::exchange(other._ptr, {})}, _data {std::exchange(other._data, {})}
    {
    }

    KRYS_NODISCARD constexpr TPtr Ptr() const noexcept
    {
      return _ptr;
    }

    constexpr void SetPtr(TPtr pointer) noexcept
    {
      _ptr = pointer;
    }

    KRYS_NODISCARD constexpr TData Data() const noexcept
    {
      return _data;
    }

    constexpr void SetData(TData data) noexcept
    {
      _data = data;
    }

    constexpr void swap(CompactPtrTuple &other) noexcept
    {
      std::swap(_ptr, other._ptr);
      std::swap(_data, other._data);
    }
#endif
  };
}
