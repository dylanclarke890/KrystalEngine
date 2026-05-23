#pragma once

#include "Krystal.Lib/Commands/CommandArena.hpp"
#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Mixins/NonCopyable.hpp"
#include "Krystal.Lib/Types/Numeric.hpp"
#include "Krystal.Lib/Types/Span.hpp"
#include <cassert>

namespace Krys
{
  /// @brief A list of commands allocated in a CommandArena.
  class CommandList : NonCopyable<CommandList>
  {
  private:
    CommandArena _arena;
    size_t _startOffset {0u};
    size_t _endOffset {0u};

  public:
    explicit CommandList(size_t initialCapacity = 32_KB) : _arena(initialCapacity)
    {
    }

    template <typename T>
    void Push(const T &command)
    {
      _arena.Allocate(command);
      _endOffset = _arena.Size();
    }

    KRYS_NODISCARD Span<const byte> GetSubSpan(size_t offset, size_t size) const noexcept
    {
      size_t spanStart = _startOffset + offset;
      assert(spanStart + size <= _endOffset && "Invalid span range");
      return _arena.GetSpan(spanStart, spanStart + size);
    }

    KRYS_NODISCARD size_t SizeInBytes() const noexcept
    {
      return _endOffset - _startOffset;
    }

    void Clear() noexcept
    {
      _arena.Clear();
      _startOffset = 0u;
      _endOffset = 0u;
    }
  };
}