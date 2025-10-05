#include "Krystal.IO/VirtualFileSystem.hpp"
#include <ranges>

namespace Krys::IO
{
  Unique<VirtualFileSystem> VirtualFileSystemBuilder::Build() noexcept
  {
    if (_mounts.empty())
    {
      return nullptr;
    }

    try
    {
      std::ranges::sort(_mounts,
                        [](const FileBackend &first, const FileBackend &second)
                        {
                          // shortest prefix first (longest would be faster but exact matches may be missed
                          // that way)
                          if (first.Prefix.Length() != second.Prefix.Length())
                          {
                            return first.Prefix.Length() < second.Prefix.Length();
                          }

                          // then by lexicographical order
                          if (first.Prefix != second.Prefix)
                          {
                            return first.Prefix.ToString() < second.Prefix.ToString();
                          }
                          return first.InsertionOrder
                                 > second.InsertionOrder; // later entries take precedence
                        });

      List<Pair<Path, Unique<IFileBackend>>> backends {};

      backends.reserve(_mounts.size());

      for (const auto &entry : _mounts)
      {
        backends.emplace_back(entry.Prefix, entry.Backend);
      }

      _mounts = {};        // Clear mounts after building to avoid dangling references.
      _insertionOrder = 0; // Reset insertion order for the next build.

      auto vfs = Unique<VirtualFileSystem>(new VirtualFileSystem());
      vfs->_backends = std::move(backends);
      return vfs;
    }
    catch (...)
    {
      return nullptr;
    }
  }

  bool VirtualFileSystem::Exists(const Path &path) const noexcept
  {
    auto allBackends = GetBackends(path);
    return std::ranges::any_of(allBackends, [](const auto &pair) { return pair.second->Exists(pair.first); });
  }

  bool VirtualFileSystem::IsDirectory(const Path &path) const noexcept
  {
    auto allBackends = GetBackends(path);
    return std::ranges::any_of(allBackends,
                               [](const auto &pair) { return pair.second->IsDirectory(pair.first); });
  }

  bool VirtualFileSystem::IsFile(const Path &path) const noexcept
  {
    auto allBackends = GetBackends(path);
    return std::ranges::any_of(allBackends, [](const auto &pair) { return pair.second->IsFile(pair.first); });
  }

  bool VirtualFileSystem::CreateFile(const Path &path, bool overwriteExisting) noexcept
  {
    auto backend = GetBackend(path);
    if (backend.has_value())
    {
      return backend->second->CreateFile(backend->first, overwriteExisting);
    }

    return false;
  }

  bool VirtualFileSystem::DeleteFile(const Path &path) noexcept
  {
    auto backend = GetBackend(path);
    if (backend.has_value())
    {
      return backend->second->DeleteFile(backend->first);
    }

    return false;
  }

  List<VirtualDirectoryEntry> VirtualFileSystem::GetDirectoryEntries(const Path &directory,
                                                                     bool recursive) const noexcept
  {
    List<VirtualDirectoryEntry> result;
    auto allBackends = GetBackends(directory);
    for (const auto &[relativePath, backend] : allBackends)
    {
      for (auto &entry : backend->GetDirectoryEntries(relativePath, recursive))
      {
        try
        {
          result.emplace_back(std::move(entry));
        }
        catch (...)
        {
          continue;
        }
      }
    }
    return result;
  }

  Unique<IStreamReader> VirtualFileSystem::GetReader(const Path &path) const noexcept
  {
    auto allBackends = GetBackends(path);
    for (const auto &pair : allBackends)
    {
      if (pair.second->Exists(pair.first))
      {
        return pair.second->GetReader(pair.first);
      }
    }
    return nullptr;
  }

  Unique<IStreamWriter> VirtualFileSystem::GetWriter(const Path &path) const noexcept
  {
    auto backend = GetBackend(path);
    if (backend.has_value() && backend->second->Exists(backend->first))
    {
      return backend->second->GetWriter(backend->first);
    }

    return nullptr;
  }

  List<Pair<Path, Ptr<IFileBackend>>> VirtualFileSystem::GetBackends(const Path &path) const noexcept
  {
    List<Pair<Path, Ptr<IFileBackend>>> result;
    for (const auto &[root, backend] : _backends)
    {
      if (path.StartsWith(root))
      {
        try
        {
          result.emplace_back(path.RelativePath(root), backend.get());
        }
        catch (...)
        {
          continue;
        }
      }
    }
    return result;
  }

  Nullable<Pair<Path, Ptr<IFileBackend>>> VirtualFileSystem::GetBackend(const Path &path) const noexcept
  {
    if (auto all = GetBackends(path); !all.empty())
    {
      return std::make_pair(all.front().first, all.front().second);
    }
    return std::nullopt;
  }
}
