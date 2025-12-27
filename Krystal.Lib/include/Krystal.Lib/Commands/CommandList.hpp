#pragma once

#include "Krystal.Lib/Commands/CommandArena.hpp"
#include "Krystal.Lib/Core/Compiler.hpp"
#include "Krystal.Lib/Types/Numeric.hpp"
#include "Krystal.Lib/Types/Span.hpp"
#include <cassert>

namespace Krys
{
  /// @brief A list of commands allocated in a CommandArena.
  class CommandList
  {
    NO_COPY(CommandList)

  private:
    CommandArena _arena;
    size_t _startOffset {0u};
    size_t _endOffset {0u};

  public:
    explicit CommandList(size_t initialCapacity = 32_KB) : _arena(initialCapacity)
    {
    }

    MOVE_SWAP(CommandList)

    template <typename T>
    void Push(const T &command)
    {
      _arena.Allocate(command);
      _endOffset = _arena.Size();
    }

    Span<const byte> GetSubSpan(size_t offset, size_t size) const noexcept
    {
      size_t spanStart = _startOffset + offset;
      assert(spanStart + size <= _endOffset && "Invalid span range");
      return _arena.GetSpan(spanStart, spanStart + size);
    }

    KRYS_NODISCARD size_t SizeInBytes() const noexcept
    {
      return _endOffset - _startOffset;
    }

    void Swap(CommandList &other) noexcept
    {
      _arena.Swap(other._arena);
      std::swap(_startOffset, other._startOffset);
      std::swap(_endOffset, other._endOffset);
    }

    void Clear() noexcept
    {
      _arena.Clear();
      _startOffset = 0u;
      _endOffset = 0u;
    }
  };
}