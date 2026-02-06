#pragma once

#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Core/Concepts.hpp"
#include "Krystal.Lib/Core/Move.hpp"
#include "Krystal.Lib/Detection/AddressSpaceBitSize.hpp"
#include "Krystal.Lib/Detection/OS.hpp"
#include "Krystal.Lib/Pointers/RefCounted/GetPtr.hpp"
#include "Krystal.Lib/Pointers/RefCounted/RawPtrTraits.hpp"
#include <cstdint>
#include <utility>

#if KRYS_OS(DARWIN)
  #include <mach/vm_param.h>
#endif

namespace Krys
{
  template <typename T>
  struct AllowCompactPointers : std::false_type
  {
  };

#define KRYS_ALLOW_COMPACT_POINTERS_TO_INCOMPLETE_TYPE(ClassName)                                            \
  template <>                                                                                                \
  struct AllowCompactPointers<ClassName *> : std::true_type                                                  \
  {                                                                                                          \
  }

#define KRYS_ALLOW_COMPACT_POINTERS_IMPL constexpr static bool allowCompactPointers = true

#define KRYS_ALLOW_COMPACT_POINTERS                                                                          \
public:                                                                                                      \
  KRYS_ALLOW_COMPACT_POINTERS_IMPL;                                                                          \
                                                                                                             \
private:                                                                                                     \
  using _forceSemicolonAfterThisMacro KRYS_UNUSED_TYPE_ALIAS = int

#define KRYS_ALLOW_STRUCT_COMPACT_POINTERS                                                                   \
public:                                                                                                      \
  KRYS_ALLOW_COMPACT_POINTERS_IMPL;                                                                          \
  using _forceSemicolonAfterThisMacro KRYS_UNUSED_TYPE_ALIAS = int

  template <typename T>
  inline constexpr bool allowCompactPointers()
  {
    if constexpr (Krys::IsTypeComplete<remove_pointer_t<T>>)
    {
      return remove_pointer_t<T>::allowCompactPointers;
    }
    else
    {
      // We want to support compact pointers to incomplete types too, so we have this fallback:
      // if a type is incomplete, AllowCompactPointers can be specialized on its pointer type,
      // in which case we'll return its value. This is mostly accomplished using the
      // KRYS_ALLOW_COMPACT_POINTERS_TO_INCOMPLETE_TYPE macro.
      return AllowCompactPointers<remove_const_t<remove_pointer_t<T>> *>::value;
    }
  }
  template <typename T>
  class CompactPtr
  {
  public:
    using StorageType = uintptr_t;
    static constexpr bool is32Bit = false;
    static constexpr bool isCompactedType = true;
    static_assert(Krys::allowCompactPointers<T *>());

    KRYS_ALWAYS_INLINE constexpr CompactPtr() = default;

    KRYS_ALWAYS_INLINE constexpr CompactPtr(std::nullptr_t)
    {
    }

    KRYS_ALWAYS_INLINE CompactPtr(T *ptr)
    {
      set(ptr);
    }

    KRYS_ALWAYS_INLINE constexpr CompactPtr(const CompactPtr &other) : _ptr(other._ptr)
    {
    }

    template <typename X>
    KRYS_ALWAYS_INLINE constexpr CompactPtr(const CompactPtr<X> &other) : _ptr(other._ptr)
    {
      static_assert(std::is_convertible_v<X *, T *>);
    }

    KRYS_ALWAYS_INLINE CompactPtr(CompactPtr &&other)
    {
      swap(other);
    }

    template <typename X>
    KRYS_ALWAYS_INLINE CompactPtr(CompactPtr<X> &&other) : _ptr(other._ptr)
    {
      static_assert(std::is_convertible_v<X *, T *>);
      std::exchange(other._ptr, 0);
    }

    KRYS_ALWAYS_INLINE constexpr CompactPtr(HashTableDeletedValueType) : _ptr(hashDeletedStorageValue)
    {
    }

    KRYS_ALWAYS_INLINE ~CompactPtr() = default;

    T &operator*() const
    {
      return *get();
    }

    KRYS_ALWAYS_INLINE T *operator->() const
    {
      return get();
    }

    bool operator!() const
    {
      return !get();
    }

    explicit operator bool() const
    {
      return !!get();
    }

    CompactPtr<T> &operator=(std::nullptr_t)
    {
      exchange(nullptr);
      return *this;
    }

    CompactPtr<T> &operator=(const CompactPtr &other)
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
    CompactPtr<T> &operator=(const CompactPtr<X> &other)
    {
      static_assert(std::is_convertible_v<X *, T *>);
      CompactPtr copy(other);
      swap(copy);
      return *this;
    }

    CompactPtr<T> &operator=(T *optr)
    {
      CompactPtr copy(optr);
      swap(copy);
      return *this;
    }

    CompactPtr<T> &operator=(CompactPtr &&other)
    {
      CompactPtr moved(WTF::move(other));
      swap(moved);
      return *this;
    }

    template <typename X>
    CompactPtr<T> &operator=(CompactPtr<X> &&other)
    {
      static_assert(std::is_convertible_v<X *, T *>);
      CompactPtr moved(WTF::move(other));
      swap(moved);
      return *this;
    }

    T *get() const
    {
      return decode(_ptr);
    }

    void set(T *ptr)
    {
      _ptr = encode(ptr);
    }

    template <class U>
    T *exchange(U &&newValue)
    {
      T *oldValue = get();
      set(std::forward<U>(newValue));
      return oldValue;
    }

    void swap(std::nullptr_t)
    {
      set(nullptr);
    }

    void swap(CompactPtr &other)
    {
      std::swap(_ptr, other._ptr);
    }

    template <typename Other>
    requires Other::isCompactedType
    void swap(Other &other)
    {
      T *t1 = get();
      T *t2 = other.get();
      set(t2);
      other.set(t1);
    }

    void swap(T *&t2)
    {
      T *t1 = get();
      std::swap(t1, t2);
      set(t1);
    }

    static KRYS_ALWAYS_INLINE StorageType encode(T *ptr)
    {
      uintptr_t intPtr = std::bit_cast<uintptr_t>(ptr);
      return intPtr;
    }

    static KRYS_ALWAYS_INLINE T *decode(StorageType ptr)
    {
      return std::bit_cast<T *>(ptr);
    }

    bool isHashTableDeletedValue() const
    {
      return _ptr == hashDeletedStorageValue;
    }

    template <typename U>
    friend bool operator==(const CompactPtr &a, const CompactPtr<U> &b)
    {
      return a._ptr == b._ptr;
    }

    StorageType storage() const
    {
      return _ptr;
    }

  private:
    template <typename X>
    friend class CompactPtr;

    static constexpr uint32_t bitsShift = 4;
    static constexpr uintptr_t alignmentMask = (1ull << bitsShift) - 1;
    static constexpr StorageType hashDeletedStorageValue =
      1; // 0x16 (encoded as 1) is within the first unmapped page for nullptr. Thus, it never appears.

    StorageType _ptr {0};
  };

  template <typename T, typename U>
  inline bool operator==(const CompactPtr<T> &a, U *b)
  {
    return a.get() == b;
  }

  template <typename T>
  struct GetPtrHelper<CompactPtr<T>>
  {
    using PtrType = T *;
    using UnderlyingType = T;
    static T *getPtr(const CompactPtr<T> &p)
    {
      return const_cast<T *>(p.get());
    }
  };

  template <typename T>
  struct IsSmartPtr<CompactPtr<T>>
  {
    static constexpr bool value = true;
    static constexpr bool isNullable = true;
  };

  template <typename T>
  struct CompactPtrTraits
  {
    template <typename U>
    using RebindTraits = RawPtrTraits<U>;

    using StorageType = CompactPtr<T>;

    static constexpr bool is32Bit = StorageType::is32Bit;

    template <typename U>
    static KRYS_ALWAYS_INLINE T *exchange(StorageType &ptr, U &&newValue)
    {
      return ptr.exchange(newValue);
    }

    template <typename Other>
    static KRYS_ALWAYS_INLINE void swap(StorageType &a, Other &b)
    {
      a.swap(b);
    }

    static KRYS_ALWAYS_INLINE T *unwrap(const StorageType &ptr)
    {
      return ptr.get();
    }

    static StorageType hashTableDeletedValue()
    {
      return StorageType {HashTableDeletedValue};
    }
    static KRYS_ALWAYS_INLINE bool isHashTableDeletedValue(const StorageType &ptr)
    {
      return ptr.isHashTableDeletedValue();
    }
  };
}
