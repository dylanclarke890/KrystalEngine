#pragma once

#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Core/Concepts.hpp"
#include "Krystal.Lib/Detection/AddressSpaceBitSize.hpp"
#include "Krystal.Lib/Detection/CPU.hpp"
#include "Krystal.Lib/Pointers/RawPtr.hpp"
#include <bit>
#include <cassert>
#include <concepts>

namespace Krys
{
  template <typename Trait, typename T>
  concept TaggingTraits =
    requires(const RawPtr<T> ptr, typename Trait::storage_type storage, typename Trait::tag_type tag) {
      { Trait::DefaultTag } -> ConvertibleTo<typename Trait::tag_type>;
      { Trait::Wrap(ptr, tag) } -> ConvertibleTo<typename Trait::storage_type>;
      { Trait::UnwrapPtr(storage) } -> ConvertibleTo<RawPtr<T>>;
      { Trait::UnwrapTag(storage) } -> ConvertibleTo<typename Trait::tag_type>;
    };

  template <typename T, TaggingTraits<T> Traits>
  class TaggedPtr
  {
  public:
    using storage_type = typename Traits::storage_type;
    using tag_type = typename Traits::tag_type;

  private:
    storage_type _ptr {Traits::Wrap(nullptr, Traits::DefaultTag)};

  public:
    constexpr TaggedPtr() noexcept = default;

    constexpr TaggedPtr(const RawPtr<T> ptr, tag_type tag = Traits::DefaultTag) noexcept
        : _ptr(Traits::Wrap(ptr, tag))
    {
    }

    KRYS_NODISCARD constexpr tag_type Tag() const noexcept
    {
      return Traits::UnwrapTag(_ptr);
    }

    KRYS_NODISCARD constexpr const RawPtr<T> Ptr() const noexcept
    {
      return Traits::UnwrapPtr(_ptr);
    }

    KRYS_NODISCARD constexpr RawPtr<T> Ptr() noexcept
    {
      return Traits::UnwrapPtr(_ptr);
    }

    constexpr void Set(const RawPtr<T> t, tag_type tag) noexcept
    {
      _ptr = Traits::Wrap(t, tag);
    }

    constexpr void SetTag(tag_type tag) noexcept
    {
      _ptr = Traits::Wrap(Ptr(), tag);
    }

    constexpr TaggedPtr &operator=(const RawPtr<T> ptr) noexcept
    {
      _ptr = Traits::Wrap(ptr, Tag());
      return *this;
    }

    constexpr const RawPtr<T> operator->() const noexcept
    {
      return Ptr();
    }

    constexpr RawPtr<T> operator->() noexcept
    {
      return Ptr();
    }
  };

  template <typename T, typename Enum, Enum DefaultEnumTag = static_cast<Enum>(0)>
  struct EnumTaggingTraits
  {
    using storage_type = uintptr_t;
    using tag_type = Enum;

    constexpr static tag_type DefaultTag = DefaultEnumTag;
    constexpr static storage_type TagMask32Bit = (1 << (alignof(remove_pointer_t<T>) - 1)) - 1;

    KRYS_NODISCARD constexpr static storage_type Wrap(const RawPtr<T> ptr, tag_type tag) noexcept
    {
      assert((static_cast<storage_type>(tag) | TagMask32Bit) == TagMask32Bit
             && "Tag is too big for 32-bit storage");
      assert(FromStorage(ToStorage(tag)) == tag);

      return std::bit_cast<storage_type>(ptr) | ToStorage(tag);
    }

#if KRYS_ADDRESS_SPACE(64)
    // Save the bottom four bits of the high byte for other uses.
    constexpr static unsigned TagShift = sizeof(storage_type) * CHAR_BIT - CHAR_BIT + 4;
    constexpr static storage_type PtrMask = (1ull << TagShift) - 1;

    KRYS_NODISCARD constexpr static RawPtr<T> UnwrapPtr(storage_type storage) noexcept
    {
      return std::bit_cast<RawPtr<T>>(storage & PtrMask);
    }
#else
    KRYS_NODISCARD constexpr static RawPtr<T> UnwrapPtr(storage_type storage) noexcept
    {
      return std::bit_cast<RawPtr<T>>(storage & ~TagMask32Bit);
    }
#endif

    KRYS_NODISCARD constexpr static tag_type UnwrapTag(storage_type storage) noexcept
    {
      return FromStorage(storage);
    }

#if KRYS_ADDRESS_SPACE(64)
    KRYS_NODISCARD constexpr static storage_type ToStorage(tag_type tag) noexcept
    {
      return static_cast<storage_type>(tag) << TagShift;
    }

    KRYS_NODISCARD constexpr static tag_type FromStorage(storage_type storage) noexcept
    {
      return static_cast<tag_type>(storage >> TagShift);
    }
#else
    KRYS_NODISCARD constexpr static storage_type ToStorage(tag_type tag) noexcept
    {
      return static_cast<storage_type>(tag);
    }

    KRYS_NODISCARD constexpr static tag_type FromStorage(storage_type storage) noexcept
    {
      return static_cast<tag_type>(storage & TagMask32Bit);
    }
#endif
  };

  // Useful for places where you sometimes want to tag and sometimes not based on template parameters.
  template <typename T>
  struct NoTaggingTraits
  {
    using storage_type = uintptr_t;
    using tag_type = unsigned;

    constexpr static tag_type DefaultTag = 0;

    static storage_type Wrap(const RawPtr<T> ptr, tag_type) noexcept
    {
      return std::bit_cast<storage_type>(ptr);
    }

    static RawPtr<T> UnwrapPtr(storage_type storage) noexcept
    {
      return std::bit_cast<RawPtr<T>>(storage);
    }

    static tag_type UnwrapTag(storage_type) noexcept
    {
      return DefaultTag;
    }
  };
}
