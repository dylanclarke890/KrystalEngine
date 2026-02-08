#pragma once

#include "Krystal.Lib/Detection/AddressSpaceBitSize.hpp"
#include "Krystal.Lib/Detection/CPU.hpp"
#include <bit>
#include <cassert>
#include <concepts>

namespace Krys
{
  template <typename Trait, typename T>
  concept TaggingTraits =
    requires(const T *ptr, typename Trait::storage_type storage, typename Trait::TagType tag) {
      { Trait::defaultTag } -> std::convertible_to<typename Trait::TagType>;
      { Trait::wrap(ptr, tag) } -> std::convertible_to<typename Trait::storage_type>;
      { Trait::unwrapPtr(storage) } -> std::convertible_to<T *>;
      { Trait::unwrapTag(storage) } -> std::convertible_to<typename Trait::TagType>;
    };

  template <typename T, TaggingTraits<T> Traits>
  class TaggedPtr
  {
  public:
    using storage_type = typename Traits::storage_type;
    using TagType = typename Traits::TagType;

    TaggedPtr() = default;
    TaggedPtr(const T *ptr, TagType tag = Traits::defaultTag) : m_ptr(Traits::wrap(ptr, tag))
    {
    }

    TagType tag() const
    {
      return Traits::unwrapTag(m_ptr);
    }
    const T *ptr() const
    {
      return Traits::unwrapPtr(m_ptr);
    }
    T *ptr()
    {
      return Traits::unwrapPtr(m_ptr);
    }

    void set(const T *t, TagType tag)
    {
      m_ptr = Traits::wrap(t, tag);
    }
    void setTag(TagType tag)
    {
      m_ptr = Traits::wrap(ptr(), tag);
    }

    TaggedPtr &operator=(const T *t)
    {
      m_ptr = Traits::wrap(t, tag());
      return *this;
    }

    const T *operator->() const
    {
      return ptr();
    }
    T *operator->()
    {
      return ptr();
    }

  private:
    storage_type m_ptr {Traits::wrap(nullptr, Traits::defaultTag)};
  };

  template <typename T, typename Enum, Enum defaultEnumTag = static_cast<Enum>(0)>
  struct EnumTaggingTraits
  {
    using storage_type = uintptr_t;
    using TagType = Enum;
    static constexpr TagType defaultTag = defaultEnumTag;

    static storage_type wrap(const T *ptr, TagType tag)
    {
      assert((static_cast<storage_type>(tag) | tagMask32Bit) == tagMask32Bit
             && "Tag is too big for 32-bit storage");
      assert(fromStorage(toStorage(tag)) == tag);
      return std::bit_cast<storage_type>(ptr) | toStorage(tag);
    }

#if KRYS_ADDRESS_SPACE(64)
    static T *unwrapPtr(storage_type storage)
    {
      return std::bit_cast<T *>(storage & ptrMask);
    }
#else
    static T *unwrapPtr(storage_type storage)
    {
      return std::bit_cast<T *>(storage & ~tagMask32Bit);
    }
#endif

    static TagType unwrapTag(storage_type storage)
    {
      return fromStorage(storage);
    }

    static constexpr storage_type tagMask32Bit = (1 << (alignof(std::remove_pointer_t<T>) - 1)) - 1;
#if KRYS_ADDRESS_SPACE(64)
    static constexpr unsigned tagShift = sizeof(storage_type) * CHAR_BIT - CHAR_BIT
                                         + 4; // Save the bottom four bits of the high byte for other uses.
    static constexpr storage_type ptrMask = (1ull << tagShift) - 1;
    static storage_type toStorage(TagType tag)
    {
      return static_cast<storage_type>(tag) << tagShift;
    }
    static TagType fromStorage(storage_type storage)
    {
      return static_cast<TagType>(storage >> tagShift);
    }
#else
    static storage_type toStorage(TagType tag)
    {
      return static_cast<storage_type>(tag);
    }
    static TagType fromStorage(storage_type storage)
    {
      return static_cast<TagType>(storage & tagMask32Bit);
    }
#endif
  };

  // Useful for places where you sometimes want to tag and sometimes not based on template parameters.
  template <typename T>
  struct NoTaggingTraits
  {
    using storage_type = uintptr_t;
    using TagType = unsigned;
    static constexpr TagType defaultTag = 0;
    static storage_type wrap(const T *ptr, TagType)
    {
      return std::bit_cast<storage_type>(ptr);
    }
    static T *unwrapPtr(storage_type storage)
    {
      return std::bit_cast<T *>(storage);
    }
    static TagType unwrapTag(storage_type)
    {
      return defaultTag;
    }
  };

  class TaggedBits60
  {
  public:
    TaggedBits60(uint64_t bits, uint8_t tag)
    {
      uint64_t bigTag = static_cast<uint64_t>(tag);
      assert((bigTag << tagShift) >> tagShift == tag);
      assert(!(bits & ~ptrMask));
      m_bits = bits | (bigTag << tagShift);
    }

    template <typename T>
    TaggedBits60(T *bits, uint8_t tag) : TaggedBits60(std::bit_cast<uintptr_t>(bits), tag)
    {
    }

    TaggedBits60(std::nullptr_t) : m_bits(0)
    {
    }

    uint64_t bits() const
    {
      return m_bits & ptrMask;
    }
    void *ptr() const
    {
      return std::bit_cast<void *>(static_cast<uintptr_t>(bits()));
    }
    uint8_t tag() const
    {
      return (m_bits & ~ptrMask) >> tagShift;
    }

  private:
    static constexpr size_t tagShift = 60;
    static constexpr uint64_t ptrMask = (1ull << tagShift) - 1;
    uint64_t m_bits;
  };
}
