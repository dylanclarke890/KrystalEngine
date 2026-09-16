#pragma once

#include "Krystal.Core/Commands/CommandArena.hpp"
#include "Krystal.Core/Commands/CommandList.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/Types/Span.hpp"
#include "Krystal.Core/Utils/ByteUtils.hpp"

namespace krys
{
  /// @brief Reads commands from a CommandList sequentially.
  class CommandListReader
  {
    KRYS_NON_COPYABLE(CommandListReader);

  private:
    const CommandList &_list;
    size_t _offset {0u};

  public:
    explicit CommandListReader(const CommandList &commandList) noexcept : _list(commandList)
    {
    }

    KRYS_NODISCARD CommandHeader ReadHeader() noexcept
    {
      const auto *data = _list.GetSubSpan(_offset, sizeof(CommandHeader)).data();
      _offset += sizeof(CommandHeader);
      return *reinterpret_cast<const CommandHeader *>(data);
    }

    template <typename T>
    KRYS_NODISCARD const T &ReadCommand() noexcept
    {
      const auto *data = _list.GetSubSpan(_offset, sizeof(T)).data();
      _offset += sizeof(T);
      _offset = ByteUtils::AlignNext(_offset, CommandArena::Alignment);
      return *reinterpret_cast<const T *>(data);
    }

    void SkipBytes(size_t byteCount) noexcept
    {
      _offset += byteCount;
      _offset = ByteUtils::AlignNext(_offset, CommandArena::Alignment);
    }

    KRYS_NODISCARD bool HasMore() const noexcept
    {
      return _offset < _list.SizeInBytes();
    }
  };
}