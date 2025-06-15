#pragma once

#include "Krystal.Core/Core.hpp"
#include "Krystal.IO/Common.hpp"
#include "Krystal.IO/IFileBackend.hpp"
#include "Krystal.IO/IStream.hpp"
#include "Krystal.IO/Path.hpp"

namespace Krys::IO
{
  class NativeFileBackend : public IFileBackend
  {
    Path _root;

  public:
    NO_COPY_MOVE(NativeFileBackend)

    explicit NativeFileBackend(const Path &root) noexcept;

    ~NativeFileBackend() noexcept override = default;

    NO_DISCARD bool Exists(const Path &path) const noexcept override;

    NO_DISCARD bool IsDirectory(const Path &path) const noexcept override;

    NO_DISCARD bool IsFile(const Path &path) const noexcept override;

    bool CreateFile(const Path &path, bool overwriteExisting) noexcept override;

    bool DeleteFile(const Path &path) noexcept override;

    NO_DISCARD List<VirtualDirectoryEntry>
      GetDirectoryEntries(const Path &directory, bool recursive = false) const noexcept override;

    NO_DISCARD Unique<IStreamReader> GetReader(const Path &path) const noexcept override;

    NO_DISCARD Unique<IStreamWriter> GetWriter(const Path &path) const noexcept override;
  };
}