#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Containers/EnumSet.hpp"
#include "Krystal.Core/Containers/List.hpp"
#include "Krystal.Core/Enum.hpp"
#include "Krystal.Core/IO/Common.hpp"
#include "Krystal.Core/IO/Path.hpp"
#include "Krystal.Core/IO/Streams/Stream.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Memory/UniquePtr.hpp"
#include "Krystal.Core/Numeric.hpp"

namespace krys::io
{
  enum class FileSearchFlag : uint8
  {
    Recursive,       // Search recursively in subdirectories
    ExcludeReadOnly, // Filter out read-only files
  };

  consteval FileSearchFlag MaxEnumValue(FileSearchFlag) noexcept
  {
    return FileSearchFlag::ExcludeReadOnly;
  }

  using FileSearchFlags = EnumSet<FileSearchFlag>;

  /// @brief Interface for file backend operations.
  class IFileBackend
  {
    KRYS_NON_COPYABLE(IFileBackend);

  protected:
    IFileBackend() = default;

  public:
    virtual ~IFileBackend() = default;

    /// @brief Checks if a file or directory exists at the specified path.
    KRYS_NODISCARD virtual bool Exists(const Path &path) const noexcept = 0;

    /// @brief Checks if the specified path is a directory.
    KRYS_NODISCARD virtual bool IsDirectory(const Path &path) const noexcept = 0;

    /// @brief Checks if the specified path is a file.
    KRYS_NODISCARD virtual bool IsFile(const Path &path) const noexcept = 0;

    /// @brief Creates a file at the specified path.
    virtual bool CreateFile(const Path &path, bool overrideExisting) = 0;

    /// @brief Deletes a file at the specified path.
    virtual bool DeleteFile(const Path &path) = 0;

    /// @brief Gets a list of files that match the specified path.
    /// @param directory The directory to search for files.
    /// @param flags Flags to control the search behavior.
    KRYS_NODISCARD virtual List<VFSFileEntry> SearchFiles(const Path &directory,
                                                          FileSearchFlags flags) const noexcept = 0;

    /// @brief Gets a stream reader for the specified path.
    KRYS_NODISCARD virtual UniquePtr<IStreamReader> GetReader(const Path &path, ReadFlags flags) const = 0;

    /// @brief Gets a stream writer for the specified path.
    KRYS_NODISCARD virtual UniquePtr<IStreamWriter> GetWriter(const Path &path, WriteFlags flags) const = 0;

    KRYS_NODISCARD virtual const Path &Root() const noexcept = 0;
  };
}
