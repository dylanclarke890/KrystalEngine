#pragma once

#include "Krystal.Gfx/Commands/CommandArena.hpp"
#include "Krystal.Gfx/Commands/CommandList.hpp"
#include "Krystal.Lib/ByteUtils.hpp"
#include "Krystal.Lib/Macros.hpp"
#include "Krystal.Lib/Span.hpp"
#include "Krystal.Lib/Types.hpp"

namespace Krys::Gfx
{
  /// @brief Reads commands from a CommandList sequentially.
  class CommandListReader
  {
    NO_COPY_MOVE(CommandListReader)

  private:
    const CommandList &_list;
    size_t _offset {0u};

  public:
    explicit CommandListReader(const CommandList &commandList) noexcept : _list(commandList)
    {
    }

    NO_DISCARD CommandHeader ReadHeader() noexcept
    {
      auto *data = _list.GetSubSpan(_offset, sizeof(CommandHeader)).data();
      _offset += sizeof(CommandHeader);
      return *reinterpret_cast<const CommandHeader *>(data);
    }

    template <typename T>
    NO_DISCARD const T &ReadCommand() noexcept
    {
      auto *data = _list.GetSubSpan(_offset, sizeof(T)).data();
      _offset += sizeof(T);
      _offset = ByteUtils::AlignNext(_offset, CommandArena::Alignment);
      return *reinterpret_cast<const T *>(data);
    }

    void SkipBytes(size_t byteCount) noexcept
    {
      _offset += byteCount;
      _offset = ByteUtils::AlignNext(_offset, CommandArena::Alignment);
    }

    NO_DISCARD bool HasMore() const noexcept
    {
      return _offset < _list.SizeInBytes();
    }
  };
}