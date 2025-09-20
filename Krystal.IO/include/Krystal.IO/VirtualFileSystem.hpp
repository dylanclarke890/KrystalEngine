#pragma once

#include "Krystal.IO/Common.hpp"
#include "Krystal.IO/IFileBackend.hpp"
#include "Krystal.IO/IStream.hpp"
#include "Krystal.IO/Path.hpp"
#include "Krystal.Lib/Attributes.hpp"
#include "Krystal.Lib/Concepts.hpp"
#include "Krystal.Lib/Macros.hpp"
#include "Krystal.Lib/Nullable.hpp"
#include "Krystal.Lib/Pair.hpp"
#include "Krystal.Lib/Types.hpp"

namespace Krys::IO
{
  class VirtualFileSystem;

  /// @brief A builder for creating a virtual file system with multiple backends.
  class VirtualFileSystemBuilder
  {
    struct FileBackend
    {
      Path Prefix;
      IFileBackend *Backend = nullptr;
      size_t InsertionOrder = 0;
    };

    List<FileBackend> _mounts {};
    size_t _insertionOrder = 0;

  public:
    NO_COPY_MOVE(VirtualFileSystemBuilder)

    VirtualFileSystemBuilder() noexcept = default;
    ~VirtualFileSystemBuilder() noexcept = default;

    /// @brief Adds a mount point to the virtual file system.
    template <DerivedFrom<IFileBackend> T, typename... Args>
    // NOLINTNEXTLINE(cppcoreguidelines-missing-std-forward)
    VirtualFileSystemBuilder &Mount(const Path &path, Args &&...args) noexcept
    {
      auto backend = CreateUnique<T>(std::forward<Args>(args)...);
      _mounts.emplace_back(FileBackend {path, backend.release(), _insertionOrder++});
      return *this;
    }

    /// @brief Adds a mount point to the virtual file system.
    template <DerivedFrom<IFileBackend> T>
    VirtualFileSystemBuilder &Mount(const Path &path, T *backend) noexcept
    {
      _mounts.emplace_back(FileBackend {path, backend});
      return *this;
    }

    /// @brief Finalizes the virtual file system and returns it.
    NO_DISCARD Unique<VirtualFileSystem> Build() noexcept;
  };

  /// @brief A virtual file system that can contain multiple backends, providing unified access to files
  /// across different storage locations. It allows for reading and writing files as if they were in a single
  /// file system, while actually accessing them through various backends like local files, cloud storage,
  /// etc.
  /// @note The last backend added will take precedence over previous ones if multiple backends match the same
  /// path. This means that if a file exists in multiple backends, the one added last will be used for reading
  /// or writing, even if earlier backends also match the path.
  class VirtualFileSystem
  {
    List<Pair<Path, Unique<IFileBackend>>> _backends {};

  public:
    NO_COPY_MOVE(VirtualFileSystem)

    ~VirtualFileSystem() noexcept = default;

    /// @returns True if the path exists in any of the configured backends.
    NO_DISCARD bool Exists(const Path &path) const noexcept;

    /// @returns True if the path exists in any of the configured backends and is a directory.
    NO_DISCARD bool IsDirectory(const Path &path) const noexcept;

    /// @returns True if the path exists in any of the configured backends and is a file.
    NO_DISCARD bool IsFile(const Path &path) const noexcept;

    /// @brief Creates a file at the specified path.
    /// @param path The path where the file should be created.
    /// @param overwriteExisting If true, overwrites the file if it already exists. If false, does nothing if
    /// the file exists.
    bool CreateFile(const Path &path, bool overwriteExisting = false) noexcept;

    /// @brief Deletes a file at the specified path.
    /// @param path The path of the file to delete.
    bool DeleteFile(const Path &path) noexcept;

    /// @brief Gets a list of files from all backends that match the specified path.
    /// @param path The directory to search for files.
    /// @param recursive If true, searches recursively in subdirectories.
    List<VirtualDirectoryEntry> GetDirectoryEntries(const Path &directory,
                                                    bool recursive = false) const noexcept;

    /// @returns Gets a stream reader for the given path. If multiple backends match, the first one is used
    /// where the file exists. If no backend matches or the file doesn't exist, returns a null pointer.
    NO_DISCARD Unique<IStreamReader> GetReader(const Path &path) const noexcept;

    /// @returns Gets a stream writer for the given path. If multiple backends match, the first one is used.
    /// If no backend matches or the file doesn't exist, returns a null pointer.
    NO_DISCARD Unique<IStreamWriter> GetWriter(const Path &path) const noexcept;

  private:
    friend class VirtualFileSystemBuilder;

    VirtualFileSystem() = default;

    /// @returns The first backend that matches the given path.
    NO_DISCARD Nullable<Pair<Path, Ptr<IFileBackend>>> GetBackend(const Path &path) const noexcept;

    /// @returns A list of all backends that match the given path.
    NO_DISCARD List<Pair<Path, Ptr<IFileBackend>>> GetBackends(const Path &path) const noexcept;
  };
}