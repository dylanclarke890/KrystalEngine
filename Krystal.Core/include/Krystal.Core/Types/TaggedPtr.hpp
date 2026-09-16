#pragma once

#include "Krystal.Core/Assert.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Concepts.hpp"
#include "Krystal.Core/Detection/AddressSpaceBitSize.hpp"
#include "Krystal.Core/Detection/CPU.hpp"
#include <bit>

namespace krys
{
  template <typename Trait, typename T>
  concept TaggingTraits =
    requires(const T *ptr, typename Trait::storage_type storage, typename Trait::tag_type tag) {
      { Trait::DefaultTag } -> ConvertibleTo<typename Trait::tag_type>;
      { Trait::Wrap(ptr, tag) } -> ConvertibleTo<typename Trait::storage_type>;
      { Trait::UnwrapPtr(storage) } -> ConvertibleTo<T *>;
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

    constexpr TaggedPtr(const T *ptr, tag_type tag = Traits::DefaultTag) noexcept
        : _ptr(Traits::Wrap(ptr, tag))
    {
    }

    KRYS_NODISCARD constexpr tag_type Tag() const noexcept
    {
      return Traits::UnwrapTag(_ptr);
    }

    KRYS_NODISCARD constexpr const T *Ptr() const noexcept
    {
      return Traits::UnwrapPtr(_ptr);
    }

    KRYS_NODISCARD constexpr T *Ptr() noexcept
    {
      return Traits::UnwrapPtr(_ptr);
    }

    constexpr void Set(const T *t, tag_type tag) noexcept
    {
      _ptr = Traits::Wrap(t, tag);
    }

    constexpr void SetTag(tag_type tag) noexcept
    {
      _ptr = Traits::Wrap(Ptr(), tag);
    }

    constexpr TaggedPtr &operator=(const T *ptr) noexcept
    {
      _ptr = Traits::Wrap(ptr, Tag());
      return *this;
    }

    constexpr const T *operator->() const noexcept
    {
      return Ptr();
    }

    constexpr T *operator->() noexcept
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
    constexpr static storage_type TagMask32Bit = (1 << (alignof(remove_ptr_t<T>) - 1)) - 1;

    KRYS_NODISCARD constexpr static storage_type Wrap(const T *ptr, tag_type tag) noexcept
    {
      krys_assert_msg((static_cast<storage_type>(tag) | TagMask32Bit) == TagMask32Bit,
                      "Tag is too big for 32-bit storage");
      krys_assert_msg(FromStorage(ToStorage(tag)) == tag, "Tag storage conversion failed");

      return std::bit_cast<storage_type>(ptr) | ToStorage(tag);
    }

#if KRYS_ADDRESS_SPACE(64)
    // Save the bottom four bits of the high byte for other uses.
    constexpr static unsigned TagShift = sizeof(storage_type) * CHAR_BIT - CHAR_BIT + 4;
    constexpr static storage_type PtrMask = (1ull << TagShift) - 1;

    KRYS_NODISCARD constexpr static T *UnwrapPtr(storage_type storage) noexcept
    {
      return std::bit_cast<T *>(storage & PtrMask);
    }
#else
    KRYS_NODISCARD constexpr static T *UnwrapPtr(storage_type storage) noexcept
    {
      return std::bit_cast<T *>(storage & ~TagMask32Bit);
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

    static storage_type Wrap(const T *ptr, tag_type) noexcept
    {
      return std::bit_cast<storage_type>(ptr);
    }

    static T *UnwrapPtr(storage_type storage) noexcept
    {
      return std::bit_cast<T *>(storage);
    }

    static tag_type UnwrapTag(storage_type) noexcept
    {
      return DefaultTag;
    }
  };
}
