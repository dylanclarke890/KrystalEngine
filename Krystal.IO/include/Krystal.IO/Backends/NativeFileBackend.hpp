#pragma once

#include "Krystal.IO/Common.hpp"
#include "Krystal.IO/IFileBackend.hpp"
#include "Krystal.IO/IStream.hpp"
#include "Krystal.IO/Path.hpp"
#include "Krystal.Lib/Attributes.hpp"
#include "Krystal.Lib/List.hpp"
#include "Krystal.Lib/Macros.hpp"
#include "Krystal.Lib/SmartPointers.hpp"

namespace Krys::IO
{
  class NativeFileBackend : public IFileBackend
  {
  private:
    Path _root;

  public:
    NO_COPY_MOVE(NativeFileBackend)

    explicit NativeFileBackend(const Path &root);

    explicit NativeFileBackend(Path &&root);

    ~NativeFileBackend() noexcept override = default;

    NO_DISCARD bool Exists(const Path &path) const noexcept override;

    NO_DISCARD bool IsDirectory(const Path &path) const noexcept override;

    NO_DISCARD bool IsFile(const Path &path) const noexcept override;

    bool CreateFile(const Path &path, bool overwriteExisting) override;

    bool DeleteFile(const Path &path) override;

    NO_DISCARD List<VirtualDirectoryEntry>
      GetDirectoryEntries(const Path &directory, bool recursive = false) const noexcept override;

    NO_DISCARD Unique<IStreamReader> GetReader(const Path &path, ReadFlags flags) const override;

    NO_DISCARD Unique<IStreamWriter> GetWriter(const Path &path, WriteFlags flags) const override;

  private:
    List<VirtualDirectoryEntry> GetDirectoryEntries(const std::filesystem::path &fullPath) const;

    List<VirtualDirectoryEntry> GetDirectoryEntriesRecursive(const std::filesystem::path &fullPath) const;
  };
}