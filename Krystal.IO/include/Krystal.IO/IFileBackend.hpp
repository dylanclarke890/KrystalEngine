#pragma once

#include "Krystal.IO/Common.hpp"
#include "Krystal.IO/IStream.hpp"
#include "Krystal.IO/Path.hpp"
#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Core/Enum.hpp"
#include "Krystal.Lib/Mixins/NonCopyMovable.hpp"
#include "Krystal.Lib/Types/List.hpp"
#include "Krystal.Lib/Types/Numeric.hpp"
#include "Krystal.Lib/Types/SmartPointers.hpp"

namespace Krys::IO
{
  enum class FileSearchFlags : uint8
  {
    None = 0,
    Recursive = 1 << 0,      // Search recursively in subdirectories
    ExcludeReadOnly = 1 << 1 // Filter out read-only files
  };

  KRYS_ENUM_FLAG_OPERATORS(FileSearchFlags)

  /// @brief Interface for file backend operations.
  class IFileBackend : NonCopyMovable<IFileBackend>
  {
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
    KRYS_NODISCARD virtual Unique<IStreamReader> GetReader(const Path &path, ReadFlags flags) const = 0;

    /// @brief Gets a stream writer for the specified path.
    KRYS_NODISCARD virtual Unique<IStreamWriter> GetWriter(const Path &path, WriteFlags flags) const = 0;

    KRYS_NODISCARD virtual const Path &Root() const noexcept = 0;
  };
}