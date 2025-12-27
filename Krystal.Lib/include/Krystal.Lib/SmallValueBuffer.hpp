#pragma once

#include <bitset>
#include <cassert>
#include <cstdint>
#include <memory>

#include "Krystal.Lib/Types/Array.hpp"
#include "Krystal.Lib/Attributes.hpp"
#include "Krystal.Lib/Types/List.hpp"
#include "Krystal.Lib/SmartPointers.hpp"
#include "Krystal.Lib/Types.hpp"

namespace Krys
{
  /// @brief Container which allows storing 32 or 64 bit integer values, whose index may never change. Values
  /// are first stored in a fixed buffer of `BufferSize` 32-bit chunks, before falling back to heap
  /// allocation.
  template <size_t BufferSize>
  class SmallValueBuffer
  {
  private:
    struct Overflow
    {
      List<uint32> Buffer;
      List<bool> WideElements;
    };

    uint16 _count {0};
    Array<uint32, BufferSize> _buffer {};
    std::bitset<BufferSize> _wideElements;
    Unique<Overflow> _overflow;

  public:
    SmallValueBuffer() noexcept = default;

    SmallValueBuffer(const SmallValueBuffer &other) noexcept
    {
      *this = other;
    }

    SmallValueBuffer(SmallValueBuffer &&other) noexcept = default;

    /// @brief Add a new element to the buffer, returning the index of the element.
    uint16 Push(uint32 value)
    {
      const auto index = _count++;
      assert(index < 4'096 && "SmallValueBuffer can only hold up to 4096 chunks");
      if (index < _buffer.size())
      {
        _buffer[index] = value;
        return index;
      }

      if (_overflow == nullptr)
      {
        _overflow = CreateUnique<SmallValueBuffer::Overflow>();
      }

      _overflow->Buffer.push_back(value);
      _overflow->WideElements.push_back(false);

      return index;
    }

    /// @brief Add a new element to the buffer, returning the index of the element.
    uint16 Push(uint64 value)
    {
      const auto lsb = static_cast<uint32>(value & 0xFF'FF'FF'FF);
      const auto msb = static_cast<uint32>(value >> 32);

      const auto lsbIndex = Push(lsb);
      MAYBE_UNUSED const auto msbIndex = Push(msb);
      assert(msbIndex < 4'096 && "SmallValueBuffer can only hold up to 4096 chunks");

      if (lsbIndex < _buffer.size())
      {
        _wideElements[lsbIndex] = true;
      }
      else
      {
        _overflow->WideElements[lsbIndex - _buffer.size()] = true;
      }
      return lsbIndex;
    }

    /// @brief Replace an existing element in the buffer with a new value. A new index may be returned, e.g.
    /// if a new value is wider than the previous.
    NO_DISCARD uint16 Replace(uint16 index, uint32 value)
    {
      if (index < _buffer.size())
      {
        _buffer[index] = value;
      }
      else
      {
        _overflow->Buffer.at(index - _buffer.size()) = value;
      }

      return index;
    }

    NO_DISCARD uint16 Replace(uint16 index, uint64 value)
    {
      const bool isWide = index < _wideElements.size() ? _wideElements[index]
                                                       : _overflow->WideElements.at(index - _buffer.size());

      if (isWide)
      {
        const auto lsb = static_cast<uint32>(value & 0xFF'FF'FF'FF);
        const auto msb = static_cast<uint32>(value >> 32);

        MAYBE_UNUSED auto lsbIndex = Replace(index, lsb);
        MAYBE_UNUSED auto msbIndex = Replace(index + 1, msb);
        return index;
      }
      else
      {
        return Push(value);
      }
    }

    /// @brief Get a value of a given width.
    uint32 GetU32(uint16 index) const
    {
      if (index < _buffer.size())
      {
        return _buffer[index];
      }
      else
      {
        return _overflow->Buffer.at(index - _buffer.size());
      }
    }

    /// @brief Get a value of a given width.
    uint64 GetU64(uint16 index) const
    {
      const auto lsb = GetU32(index);
      const auto msb = GetU32(index + 1);
      return (static_cast<uint64_t>(msb) << 32) | lsb;
    }

    SmallValueBuffer &operator=(const SmallValueBuffer &other)
    {
      _count = other._count;
      _buffer = other._buffer;
      _wideElements = other._wideElements;
      _overflow = other._overflow ? CreateUnique<Overflow>(*other._overflow) : nullptr;
      return *this;
    }

    SmallValueBuffer &operator=(SmallValueBuffer &&other) noexcept = default;
  };
}