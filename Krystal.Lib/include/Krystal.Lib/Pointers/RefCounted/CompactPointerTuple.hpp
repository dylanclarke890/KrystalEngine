#pragma once

#include "Krystal.Lib/Core/Concepts.hpp"
#include "Krystal.Lib/Core/TypeTraits.hpp"
#include "Krystal.Lib/Detection/AddressSpaceBitSize.hpp"
#include "Krystal.Lib/Detection/CPU.hpp"
#include "Krystal.Lib/Detection/OS.hpp"
#include "Krystal.Lib/Pointers/RefCounted/CompactPtr.hpp"
#include "Krystal.Lib/Types/Numeric.hpp"
#include <bit>
#include <cassert>
#include <utility>

#if KRYS_OS(DARWIN)
  #include <mach/vm_param.h>
#endif

namespace Krys
{
  // The goal of this class is folding a pointer and 2 bytes value into 8 bytes in both 32bit and 64bit
  // architectures. 32bit architecture just has a pair of byte and pointer, which should be 8 bytes. We are
  // assuming 48bit pointers here, which is also assumed in JSValue anyway.
  template <typename PointerType, typename Type>
  class CompactPointerTuple final
  {
  public:
    static_assert(sizeof(Type) <= 2);
    static_assert(IsPointer<PointerType>);
    static_assert(Krys::allowCompactPointers<PointerType>());
    static_assert(Integral<Type> || IsEnum<Type>);
    using UnsignedType = make_unsigned_t<conditional_t<SameType<Type, bool>, uint8, Type>>;
    static_assert(sizeof(UnsignedType) == sizeof(Type));

    CompactPointerTuple() = default;

    friend bool operator==(const CompactPointerTuple &, const CompactPointerTuple &) = default;

#if KRYS_ADDRESS_SPACE(64)
  public:
    static constexpr unsigned maxNumberOfBitsInPointer = 48;
    static_assert(KRYS_EFFECTIVE_ADDRESS_WIDTH <= maxNumberOfBitsInPointer);

  #if KRYS_CPU(LITTLE_ENDIAN)
    static constexpr ptrdiff_t offsetOfType()
    {
      return maxNumberOfBitsInPointer / 8;
    }
  #endif

    static constexpr uint64_t pointerMask = (1ULL << maxNumberOfBitsInPointer) - 1;

    CompactPointerTuple(PointerType pointer, Type type) : m_data(encode(pointer, type))
    {
      assert(this->type() == type);
      assert(this->pointer() == pointer);
    }

    template <typename OtherPointerType>
    requires(IsPointer<PointerType> && ConvertibleTo<OtherPointerType, PointerType>)
    CompactPointerTuple(CompactPointerTuple<OtherPointerType, Type> &&other)
        : m_data {std::exchange(other.m_data, {})}
    {
    }

    PointerType pointer() const
    {
      return std::bit_cast<PointerType>(m_data & pointerMask);
    }
    void setPointer(PointerType pointer)
    {
      m_data = encode(pointer, type());
      assert(this->pointer() == pointer);
    }

    Type type() const
    {
      return decodeType(m_data);
    }
    void setType(Type type)
    {
      m_data = encode(pointer(), type);
      assert(this->type() == type);
    }

    uint64_t data() const
    {
      return m_data;
    }

    void swap(CompactPointerTuple &other)
    {
      std::swap(m_data, other.m_data);
    }

  private:
    static constexpr uint64_t encodeType(Type type)
    {
      return static_cast<uint64_t>(static_cast<UnsignedType>(type)) << maxNumberOfBitsInPointer;
    }
    static constexpr Type decodeType(uint64_t value)
    {
      return static_cast<Type>(static_cast<UnsignedType>(value >> maxNumberOfBitsInPointer));
    }

    static uint64_t encode(PointerType pointer, Type type)
    {
      return std::bit_cast<uint64_t>(pointer) | encodeType(type);
    }

    uint64_t m_data {0};
#else
  public:
    CompactPointerTuple(PointerType pointer, Type type) : m_pointer(pointer), m_type(type)
    {
    }

    template <typename OtherPointerType>
    requires(std::is_pointer_v<PointerType> && std::is_convertible_v<OtherPointerType, PointerType>)
    CompactPointerTuple(CompactPointerTuple<OtherPointerType, Type> &&other)
        : m_pointer {std::exchange(other.m_pointer, {})}, m_type {std::exchange(other.m_type, {})}
    {
    }

    PointerType pointer() const
    {
      return m_pointer;
    }
    void setPointer(PointerType pointer)
    {
      m_pointer = pointer;
    }
    Type type() const
    {
      return m_type;
    }
    void setType(Type type)
    {
      m_type = type;
    }

    void swap(CompactPointerTuple &other)
    {
      std::swap(m_pointer, other.m_pointer);
      std::swap(m_type, other.m_type);
    }

  private:
    PointerType m_pointer {nullptr};
    Type m_type {0};
#endif

    template <typename, typename>
    friend class CompactPointerTuple;
  };
}
