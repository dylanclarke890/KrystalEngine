#pragma once

#include "Krystal.IO/Path.hpp"
#include "Krystal.Lib/Core/Macros.hpp"
#include "Krystal.Lib/Types/Numeric.hpp"

namespace Krys::IO
{
  enum class ReadFlags : uint8
  {
    None = 0,
    OpenAtEnd = 1 << 0 // Open the file and move the read position to the end
  };

  ENUM_BITWISE_OPERATORS(ReadFlags)

  enum class WriteFlags : uint8
  {
    None = 0,
    Create = 1 << 0,   // Create the file if it does not exist
    Truncate = 1 << 1, // Truncate the file to zero length if it already exists
    OpenAtEnd = 1 << 2 // Open the file and move the write position to the end
  };

  ENUM_BITWISE_OPERATORS(WriteFlags)

  /// @brief Specifies the reference point used to obtain the new position in a stream.
  enum class SeekOrigin : uint8
  {
    /// @brief The beginning of a stream.
    Begin = 0,
    /// @brief The current position within a stream.
    Current = 1,
    /// @brief The end of a stream.
    End = 2
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