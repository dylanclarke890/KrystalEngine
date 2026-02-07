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
    requires(const T *ptr, typename Trait::StorageType storage, typename Trait::TagType tag) {
      { Trait::defaultTag } -> std::convertible_to<typename Trait::TagType>;
      { Trait::wrap(ptr, tag) } -> std::convertible_to<typename Trait::StorageType>;
      { Trait::unwrapPtr(storage) } -> std::convertible_to<T *>;
      { Trait::unwrapTag(storage) } -> std::convertible_to<typename Trait::TagType>;
    };

  template <typename T, TaggingTraits<T> Traits>
  class TaggedPtr
  {
  public:
    using StorageType = typename Traits::StorageType;
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
    StorageType m_ptr {Traits::wrap(nullptr, Traits::defaultTag)};
  };

  template <typename T, typename Enum, Enum defaultEnumTag = static_cast<Enum>(0)>
  struct EnumTaggingTraits
  {
    using StorageType = uintptr_t;
    using TagType = Enum;
    static constexpr TagType defaultTag = defaultEnumTag;

    static StorageType wrap(const T *ptr, TagType tag)
    {
      assert((static_cast<StorageType>(tag) | tagMask32Bit) == tagMask32Bit
             && "Tag is too big for 32-bit storage");
      assert(fromStorage(toStorage(tag)) == tag);
      return std::bit_cast<StorageType>(ptr) | toStorage(tag);
    }

#if KRYS_ADDRESS_SPACE(64)
    static T *unwrapPtr(StorageType storage)
    {
      return std::bit_cast<T *>(storage & ptrMask);
    }
#else
    static T *unwrapPtr(StorageType storage)
    {
      return std::bit_cast<T *>(storage & ~tagMask32Bit);
    }
#endif

    static TagType unwrapTag(StorageType storage)
    {
      return fromStorage(storage);
    }

    static constexpr StorageType tagMask32Bit = (1 << (alignof(std::remove_pointer_t<T>) - 1)) - 1;
#if KRYS_ADDRESS_SPACE(64)
    static constexpr unsigned tagShift = sizeof(StorageType) * CHAR_BIT - CHAR_BIT
                                         + 4; // Save the bottom four bits of the high byte for other uses.
    static constexpr StorageType ptrMask = (1ull << tagShift) - 1;
    static StorageType toStorage(TagType tag)
    {
      return static_cast<StorageType>(tag) << tagShift;
    }
    static TagType fromStorage(StorageType storage)
    {
      return static_cast<TagType>(storage >> tagShift);
    }
#else
    static StorageType toStorage(TagType tag)
    {
      return static_cast<StorageType>(tag);
    }
    static TagType fromStorage(StorageType storage)
    {
      return static_cast<TagType>(storage & tagMask32Bit);
    }
#endif
  };

  // Useful for places where you sometimes want to tag and sometimes not based on template parameters.
  template <typename T>
  struct NoTaggingTraits
  {
    using StorageType = uintptr_t;
    using TagType = unsigned;
    static constexpr TagType defaultTag = 0;
    static StorageType wrap(const T *ptr, TagType)
    {
      return std::bit_cast<StorageType>(ptr);
    }
    static T *unwrapPtr(StorageType storage)
    {
      return std::bit_cast<T *>(storage);
    }
    static TagType unwrapTag(StorageType)
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
