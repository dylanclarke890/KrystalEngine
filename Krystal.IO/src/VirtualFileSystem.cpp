#include "Krystal.IO/VirtualFileSystem.hpp"
#include <algorithm>

namespace Krys::IO
{
  Unique<VirtualFileSystem> VirtualFileSystemBuilder::Build() noexcept
  {
    if (_mounts.empty())
    {
      return nullptr;
    }

    std::sort(_mounts.begin(), _mounts.end(),
              [](const FileBackend &a, const FileBackend &b)
              {
                // shortest prefix first (longest would be faster but exact matches may be missed that way)
                if (a.Prefix.Length() != b.Prefix.Length())
                  return a.Prefix.Length() < b.Prefix.Length();
                // then by lexicographical order
                if (a.Prefix != b.Prefix)
                  return a.Prefix.ToString() < b.Prefix.ToString();
                return a.InsertionOrder > b.InsertionOrder; // later entries take precedence
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

  bool VirtualFileSystem::Exists(const Path &path) const noexcept
  {
    auto allBackends = GetBackends(path);
    for (const auto &pair : allBackends)
    {
      if (pair.second->Exists(pair.first))
        return true;
    }
    return false;
  }

  bool VirtualFileSystem::IsDirectory(const Path &path) const noexcept
  {
    auto allBackends = GetBackends(path);
    for (const auto &pair : allBackends)
    {
      if (pair.second->IsDirectory(pair.first))
        return true;
    }
    return false;
  }

  bool VirtualFileSystem::IsFile(const Path &path) const noexcept
  {
    auto allBackends = GetBackends(path);
    for (const auto &pair : allBackends)
    {
      if (pair.second->IsFile(pair.first))
        return true;
    }
    return false;
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
        result.emplace_back(std::move(entry));
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
        return pair.second->GetReader(pair.first);
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
        result.emplace_back(path.RelativePath(root), backend.get());
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
