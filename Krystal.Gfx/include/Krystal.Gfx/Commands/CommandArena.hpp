#pragma once

#include "Krystal.Gfx/Commands/CommandType.hpp"
#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/ByteUtils.hpp"
#include "Krystal.Lib/Core/Concepts.hpp"
#include "Krystal.Lib/Types/List.hpp"
#include "Krystal.Lib/Core/Macros.hpp"
#include "Krystal.Lib/Types/Span.hpp"
#include "Krystal.Lib/Types/Numeric.hpp"

namespace Krys::Gfx
{
  struct CommandHeader
  {
    CommandType Type {0u};
    uint32 SizeInBytes {0u};
  };

  /// @brief An arena for allocating command data. Commands must be standard layout types (PODs) and define a
  /// static constexpr 'CommandType' Type member.
  ///
  /// NOTE: CommandList's should only be used sequentially, you should not attempt to start a new command
  /// list while you are still pushing commands to a previous one.
  /// @code
  /// struct Command
  /// {
  ///   constexpr static CommandType Type = KRYS_CMD_TYPE("MyCommand");
  ///   Maths::Vec4 Data {};
  /// };
  /// @endcode
  class CommandArena
  {
    NO_COPY(CommandArena)

  public:
    constexpr static size_t Alignment = alignof(std::max_align_t);

  private:
    List<byte> _arena;

  public:
    CommandArena(size_t initialCapacity = 32_KB)
    {
      _arena.reserve(initialCapacity);
    }

    MOVE_SWAP(CommandArena)

    template <HasStandardLayout T>
    void Allocate(const T &command)
    {
      CommandHeader header {T::Type, sizeof(T)};
      _arena.insert(_arena.end(), reinterpret_cast<const byte *>(&header),
                    reinterpret_cast<const byte *>(&header) + sizeof(CommandHeader));

      _arena.insert(_arena.end(), reinterpret_cast<const byte *>(&command),
                    reinterpret_cast<const byte *>(&command) + sizeof(T));

      size_t alignedOffset = ByteUtils::AlignNext(_arena.size(), Alignment);
      if (alignedOffset > _arena.size())
      {
        _arena.insert(_arena.end(), alignedOffset - _arena.size(), byte {0u});
      }
    }

    NO_DISCARD size_t Size() const noexcept
    {
      return _arena.size();
    }

    NO_DISCARD Span<const byte> GetSpan(size_t start, size_t end) const noexcept
    {
      return Span<const byte>(_arena.data() + start, end - start);
    }

    void Clear() noexcept
    {
      _arena.clear();
    }

    void Swap(CommandArena &other) noexcept
    {
      _arena.swap(other._arena);
    }
  };
}