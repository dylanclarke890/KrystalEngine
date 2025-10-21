#pragma once

#include "Krystal.Gfx/Handle.hpp"
#include "Krystal.Lib/ByteUtils.hpp"
#include "Krystal.Lib/List.hpp"
#include "Krystal.Lib/Macros.hpp"
#include "Krystal.Lib/Span.hpp"
#include "Krystal.Lib/Types.hpp"

namespace Krys::Gfx
{
  enum class CommandType : uint8
  {
    Rect
  };

  struct CommandHeader
  {
    CommandType Type {};
    uint32 SizeInBytes {0u};
  };

  struct RectCommand
  {
    constexpr static CommandType CommandType = CommandType::Rect;
  };

  class CommandArena
  {
    NO_COPY_MOVE(CommandArena)

  public:
    constexpr static size_t Alignment = alignof(std::max_align_t);

  private:
    List<byte> _arena;

  public:
    CommandArena(size_t initialCapacity = 64_KB)
    {
      _arena.reserve(initialCapacity);
    }

    template <typename T>
    void Allocate(const T &command) noexcept
    {
      static_assert(std::is_standard_layout_v<T>, "Command must be a standard layout type");

      size_t alignedOffset = ByteUtils::AlignNext(_arena.size(), Alignment);
      if (alignedOffset > _arena.size())
      {
        _arena.insert(_arena.end(), alignedOffset - _arena.size(), byte {0u});
      }

      CommandHeader header {T::CommandType, sizeof(T)};
      _arena.insert(_arena.end(), reinterpret_cast<const byte *>(&header),
                    reinterpret_cast<const byte *>(&header) + sizeof(CommandHeader));

      _arena.insert(_arena.end(), reinterpret_cast<const byte *>(&command),
                    reinterpret_cast<const byte *>(&command) + sizeof(T));
    }

    size_t Size() const noexcept
    {
      return _arena.size();
    }

    Span<byte> GetSpan(size_t start, size_t end) noexcept
    {
      return Span<byte>(_arena.data() + start, end - start);
    }

    void Reset() noexcept
    {
      _arena.clear();
    }
  };

  class CommandList
  {
  private:
    CommandArena &_arena;
    size_t _startOffset {0u};
    size_t _endOffset {0u};

  public:
    CommandList(CommandArena &arena) noexcept
        : _arena(arena), _startOffset(arena.Size()), _endOffset(arena.Size())
    {
    }

    template <typename T>
    void Push(const T &command) noexcept
    {
      _arena.Allocate(command);
      _endOffset = _arena.Size();
    }

    Span<byte> GetSpan() noexcept
    {
      return _arena.GetSpan(_startOffset, _endOffset);
    }
  };
}