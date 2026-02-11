#pragma once

#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Core/Concepts.hpp"
#include "Krystal.Lib/Core/Move.hpp"
#include "Krystal.Lib/Core/TypeTraits.hpp"
#include "Krystal.Lib/Detection/AddressSpaceBitSize.hpp"
#include "Krystal.Lib/Detection/OS.hpp"
#include "Krystal.Lib/Pointers/RawPtr.hpp"
#include "Krystal.Lib/Types/Numeric.hpp"
#include <bit>
#include <cstdint>
#include <utility>

#if KRYS_OS(DARWIN)
  #include <mach/vm_param.h>
#endif

namespace Krys
{
  template <typename T>
  inline constexpr bool EnableCompactPointers = false;

#define KRYS_ENABLE_COMPACT_POINTERS_FOR_INCOMPLETE_TYPE(T)                                                  \
  template <>                                                                                                \
  inline constexpr bool EnableCompactPointers<RawPtr<T>> = true

#define KRYS_ENABLE_COMPACT_POINTERS_IMPL constexpr static bool EnableCompactPointers = true

#define KRYS_ENABLE_COMPACT_POINTERS                                                                         \
public:                                                                                                      \
  KRYS_ENABLE_COMPACT_POINTERS_IMPL;                                                                         \
                                                                                                             \
private:                                                                                                     \
  using _forceSemicolonAllowCompactPointers = int

#define KRYS_ALLOW_STRUCT_COMPACT_POINTERS                                                                   \
public:                                                                                                      \
  KRYS_ALLOW_COMPACT_POINTERS_IMPL;                                                                          \
  using _forceSemicolonAllowCompactPointers = int

  template <typename T>
  inline constexpr bool AllowsCompactPointers() noexcept
  {
    if constexpr (Krys::IsTypeComplete<remove_pointer_t<T>>)
    {
      return remove_pointer_t<T>::EnableCompactPointers;
    }
    else
    {
      // We want to support compact pointers to incomplete types too, so we have this fallback:
      // if a type is incomplete, AllowCompactPointers can be specialized on its pointer type,
      // in which case we'll return its value. This is mostly accomplished using the
      // KRYS_ENABLE_COMPACT_POINTERS_FOR_INCOMPLETE_TYPE macro.
      return EnableCompactPointers<RawPtr<remove_const_t<remove_pointer_t<T>>>>;
    }
  }
  template <typename T>
  class CompactPtr
  {
    static_assert(Krys::AllowsCompactPointers<RawPtr<T>>());

    template <typename X>
    friend class CompactPtr;

  public:
    using storage_type = uintptr_t;

    static constexpr bool Is32Bit = false;
    static constexpr bool IsCompactedType = true;

  private:
    // 0x16 (encoded as 1) is within the first unmapped page for nullptr. Thus, it never appears.
    static constexpr storage_type DeletedStorageValue = 1;

    storage_type _ptr {0};

  public:
    KRYS_ALWAYS_INLINE constexpr CompactPtr() noexcept = default;
    KRYS_ALWAYS_INLINE constexpr ~CompactPtr() noexcept = default;

    KRYS_ALWAYS_INLINE constexpr CompactPtr(std::nullptr_t) noexcept
    {
    }

    KRYS_ALWAYS_INLINE constexpr CompactPtr(RawPtr<T> ptr) noexcept
    {
      set(ptr);
    }

    KRYS_ALWAYS_INLINE constexpr CompactPtr(const CompactPtr &other) noexcept : _ptr(other._ptr)
    {
    }

    template <typename X>
    KRYS_ALWAYS_INLINE constexpr CompactPtr(const CompactPtr<X> &other) noexcept : _ptr(other._ptr)
    {
      static_assert(ConvertibleTo<RawPtr<X>, RawPtr<T>>);
    }

    KRYS_ALWAYS_INLINE constexpr CompactPtr(CompactPtr &&other) noexcept
    {
      swap(other);
    }

    template <typename X>
    KRYS_ALWAYS_INLINE constexpr CompactPtr(CompactPtr<X> &&other) noexcept : _ptr(other._ptr)
    {
      static_assert(ConvertibleTo<RawPtr<X>, RawPtr<T>>);
      std::exchange(other._ptr, 0);
    }

    constexpr T &operator*() const noexcept
    {
      return *get();
    }

    KRYS_ALWAYS_INLINE constexpr RawPtr<T> operator->() const noexcept
    {
      return get();
    }

    constexpr bool operator!() const noexcept
    {
      return !get();
    }

    explicit constexpr operator bool() const noexcept
    {
      return !!get();
    }

    constexpr CompactPtr<T> &operator=(std::nullptr_t) noexcept
    {
      exchange(nullptr);
      return *this;
    }

    constexpr CompactPtr<T> &operator=(const CompactPtr &other) noexcept
    {
      if (&other == this)
      {
        return *this;
      }

      CompactPtr copy(other);
      swap(copy);
      return *this;
    }

    template <typename X>
    constexpr CompactPtr<T> &operator=(const CompactPtr<X> &other) noexcept
    {
      static_assert(ConvertibleTo<RawPtr<X>, RawPtr<T>>);
      CompactPtr copy(other);
      swap(copy);
      return *this;
    }

    constexpr CompactPtr<T> &operator=(T *optr) noexcept
    {
      CompactPtr copy(optr);
      swap(copy);
      return *this;
    }

    constexpr CompactPtr<T> &operator=(CompactPtr &&other) noexcept
    {
      CompactPtr moved(Krys::Move(other));
      swap(moved);
      return *this;
    }

    template <typename X>
    constexpr CompactPtr<T> &operator=(CompactPtr<X> &&other) noexcept
    {
      static_assert(ConvertibleTo<RawPtr<X>, RawPtr<T>>);
      CompactPtr moved(Krys::Move(other));
      swap(moved);
      return *this;
    }

    constexpr RawPtr<T> get() const noexcept
    {
      return Decode(_ptr);
    }

    constexpr void set(RawPtr<T> ptr) noexcept
    {
      _ptr = Encode(ptr);
    }

    template <class U>
    constexpr RawPtr<T> exchange(U &&newValue) noexcept
    {
      RawPtr<T> oldValue = get();
      set(std::forward<U>(newValue));
      return oldValue;
    }

    constexpr void swap(std::nullptr_t) noexcept
    {
      set(nullptr);
    }

    constexpr void swap(CompactPtr &other) noexcept
    {
      std::swap(_ptr, other._ptr);
    }

    template <typename TOther>
    requires(TOther::IsCompactedType)
    constexpr void swap(TOther &other) noexcept
    {
      RawPtr<T> t1 = get();
      RawPtr<T> t2 = other.get();
      set(t2);
      other.set(t1);
    }

    constexpr void swap(RawPtr<T> &t2) noexcept
    {
      RawPtr<T> t1 = get();
      std::swap(t1, t2);
      set(t1);
    }

    constexpr static KRYS_ALWAYS_INLINE storage_type Encode(RawPtr<T> ptr) noexcept
    {
      uintptr_t intPtr = std::bit_cast<uintptr_t>(ptr);
      return intPtr;
    }

    constexpr static KRYS_ALWAYS_INLINE RawPtr<T> Decode(storage_type ptr) noexcept
    {
      return std::bit_cast<RawPtr<T>>(ptr);
    }

    template <typename U>
    friend constexpr bool operator==(const CompactPtr &a, const CompactPtr<U> &b) noexcept
    {
      return a._ptr == b._ptr;
    }

    constexpr storage_type GetStorage() const noexcept
    {
      return _ptr;
    }
  };

  template <typename T, typename U>
  constexpr inline bool operator==(const CompactPtr<T> &a, U *b) noexcept
  {
    return a.get() == b;
  }

  template <typename T>
  struct CompactPtrTraits
  {
    using storage_type = CompactPtr<T>;

    static constexpr bool Is32Bit = storage_type::Is32Bit;

    template <typename U>
    KRYS_ALWAYS_INLINE constexpr static RawPtr<T> exchange(storage_type &ptr, U &&newValue) noexcept
    {
      return ptr.exchange(newValue);
    }

    template <typename Other>
    KRYS_NODISCARD KRYS_ALWAYS_INLINE constexpr static void swap(storage_type &a, Other &b) noexcept
    {
      a.swap(b);
    }

    KRYS_NODISCARD KRYS_ALWAYS_INLINE constexpr static RawPtr<T> unwrap(const storage_type &ptr) noexcept
    {
      return ptr.get();
    }
  };

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
