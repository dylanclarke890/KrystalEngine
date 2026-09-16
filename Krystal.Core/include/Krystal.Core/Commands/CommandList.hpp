#pragma once

#include "Krystal.Core/Assert.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Commands/CommandArena.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/Types/Span.hpp"

namespace krys
{
  /// @brief A list of commands allocated in a CommandArena.
  class CommandList
  {
    KRYS_NON_COPYABLE(CommandList);

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
      krys_assert_msg(spanStart + size <= _endOffset, "Invalid span range");

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