#pragma once

#include "Krystal.Core/Containers/EnumSet.hpp"
#include "Krystal.Core/Enum.hpp"
#include "Krystal.Core/IO/Path.hpp"
#include "Krystal.Core/Numeric.hpp"

namespace krys::io
{
  enum class ReadFlag : uint8
  {
    OpenAtEnd, // Open the file and move the read position to the end
  };

  consteval ReadFlag MaxEnumValue(ReadFlag) noexcept
  {
    return ReadFlag::OpenAtEnd;
  }

  using ReadFlags = EnumSet<ReadFlag>;

  enum class WriteFlag : uint8
  {
    Create,    // Create the file if it does not exist
    Truncate,  // Truncate the file to zero length if it already exists
    OpenAtEnd, // Open the file and move the write position to the end
  };

  consteval WriteFlag MaxEnumValue(WriteFlag) noexcept
  {
    return WriteFlag::OpenAtEnd;
  }

  using WriteFlags = EnumSet<WriteFlag>;

  /// @brief Specifies the reference point used to obtain the new position in a stream.
  enum class SeekOrigin : uint8
  {
    /// @brief The beginning of a stream.
    Begin = 0,
    /// @brief The current position within a stream.
    Current = 1,
    /// @brief The end of a stream.
    End = 2,
  };

  struct VFSFileEntry
  {
    /// @brief The absolute path to the file in the backend.
    Path AbsolutePath;

    /// @brief The relative path to the file.
    Path RelativePath;

    /// @brief The size of the file in bytes.
    uint64 Size = 0;

    /// @brief True if the entry is not writable.
    bool IsReadOnly = false;
  };
}
