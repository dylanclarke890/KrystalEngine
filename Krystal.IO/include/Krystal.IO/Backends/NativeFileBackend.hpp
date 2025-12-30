#pragma once

#include "Krystal.IO/Common.hpp"
#include "Krystal.IO/IFileBackend.hpp"
#include "Krystal.IO/Streams/Stream.hpp"
#include "Krystal.IO/Path.hpp"
#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Core/TypeTraits.hpp"
#include "Krystal.Lib/Types/List.hpp"
#include "Krystal.Lib/Types/Numeric.hpp"
#include "Krystal.Lib/Pointers/UniquePtr.hpp"

namespace Krys::IO
{
  class NativeFileBackend : public IFileBackend
  {
  private:
    Path _root;

  public:
    explicit NativeFileBackend(const Path &root);

    explicit NativeFileBackend(Path &&root);

    ~NativeFileBackend() noexcept override = default;

    KRYS_NODISCARD bool Exists(const Path &path) const noexcept override;

    KRYS_NODISCARD bool IsDirectory(const Path &path) const noexcept override;

    KRYS_NODISCARD bool IsFile(const Path &path) const noexcept override;

    bool CreateFile(const Path &path, bool overwriteExisting) override;

    bool DeleteFile(const Path &path) override;

    KRYS_NODISCARD List<VFSFileEntry> SearchFiles(const Path &directory,
                                                  FileSearchFlags flags) const noexcept override;

    KRYS_NODISCARD UniquePtr<IStreamReader> GetReader(const Path &path, ReadFlags flags) const override;

    KRYS_NODISCARD UniquePtr<IStreamWriter> GetWriter(const Path &path, WriteFlags flags) const override;

    KRYS_NODISCARD const Path &Root() const noexcept override
    {
      return _root;
    }

  private:
    template <bool Recursive>
    List<VFSFileEntry> SearchFiles(const std::filesystem::path &path, FileSearchFlags flags) const
    {
      namespace fs = std::filesystem;
      using DirectoryIterator =
        Conditional<Recursive, fs::recursive_directory_iterator, fs::directory_iterator>;

      List<VFSFileEntry> entries;
      for (const auto &entry : DirectoryIterator(path))
      {
        if (!entry.is_regular_file())
        {
          continue;
        }

        fs::perms permissions = entry.status().permissions();
        const bool isReadOnly = (permissions & fs::perms::owner_write) == fs::perms::none;
        if (isReadOnly && !!(flags & FileSearchFlags::ExcludeReadOnly))
        {
          continue;
        }

        const Path absolutePath = Path(entry.path());
        const Path relativePath = absolutePath.RelativePath(_root);
        const auto size = static_cast<uint64>(entry.file_size());

        entries.emplace_back(absolutePath, relativePath, size, isReadOnly);
      }
      return entries;
    }
  };
}