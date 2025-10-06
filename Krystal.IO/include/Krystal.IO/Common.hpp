#pragma once

#include "Krystal.IO/Path.hpp"
#include "Krystal.Lib/Types.hpp"

namespace Krys::IO
{
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

  /// @brief Represents a virtual directory entry.
  struct VirtualDirectoryEntry
  {
    VirtualDirectoryEntry(const Path &path, size_t size, bool isDirectory)
        : Path(path), Size(size), IsDirectory(isDirectory)
    {
    }

    /// @brief The path to the file or directory.
    Path Path;

    /// @brief The size of the file in bytes, or 0 if it is a directory.
    size_t Size = 0;

    /// @brief True if this entry is a directory, false if it is a file.
    bool IsDirectory = false;
  };
}