#include "Krystal.IO/VirtualFileSystem.hpp"
#include "Krystal.Lib/DebugBreak.hpp"
#include "Krystal.Lib/Set.hpp"
#include "Krystal.Log/ILogger.hpp"
#include <ranges>

namespace Krys::IO
{
  VirtualFileSystemBuilder::~VirtualFileSystemBuilder() noexcept
  {
    for (const auto &mount : _mounts)
    {
      if (mount.Backend == nullptr)
      {
        continue;
      }

      // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
      delete mount.Backend; // Clean up any allocated backends that were not used.
    }
    _mounts.clear();
  }

  Unique<VirtualFileSystem> VirtualFileSystemBuilder::Build()
  {
    if (_mounts.empty())
    {
      return nullptr;
    }

    const auto sortFn = [](const FileBackend &first, const FileBackend &second)
    {
      const auto componentCount = [](const std::filesystem::path &p)
      {
        return static_cast<size_t>(std::distance(p.begin(), p.end()));
      };

      const size_t firstComponentsCount = componentCount(first.Prefix.NativePath());
      const size_t secondComponentsCount = componentCount(second.Prefix.NativePath());

      // longest (most components) first
      if (firstComponentsCount != secondComponentsCount)
      {
        return firstComponentsCount > secondComponentsCount;
      }

      // lexicographically smaller first
      if (first.Prefix != second.Prefix)
      {
        return first.Prefix.ToString() < second.Prefix.ToString();
      }

      return first.InsertionOrder > second.InsertionOrder; // later mounts win
    };

    std::ranges::sort(_mounts, sortFn);

    List<Pair<Path, Unique<IFileBackend>>> backends {};
    backends.reserve(_mounts.size());
    for (auto &entry : _mounts)
    {
      backends.emplace_back(entry.Prefix, Unique<IFileBackend>(entry.Backend));
      entry.Backend = nullptr; // prevent double delete
    }

    _mounts.clear();     // Clear mounts after building to avoid dangling references.
    _insertionOrder = 0; // Reset insertion order for the next build.

    auto vfs = Unique<VirtualFileSystem>(new VirtualFileSystem());
    vfs->_backends = std::move(backends);
    return vfs;
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

  Unique<IStreamReader> VirtualFileSystem::GetReader(const Path &path, ReadFlags flags) const noexcept
  {
    auto allBackends = GetBackends(path);
    for (const auto &pair : allBackends)
    {
      if (pair.second->Exists(pair.first))
      {
        return pair.second->GetReader(pair.first, flags);
      }
    }
    return nullptr;
  }

  Unique<IStreamWriter> VirtualFileSystem::GetWriter(const Path &path, WriteFlags flags) const noexcept
  {
    auto allBackends = GetBackends(path);
    for (const auto &pair : allBackends)
    {
      auto writer = pair.second->GetWriter(pair.first, flags);
      if (writer != nullptr)
      {
        return writer;
      }
    }

    return nullptr;
  }

  bool VirtualFileSystem::CreateFile(const Path &path, bool overwriteExisting) noexcept
  {
    if (auto backend = GetBackend(path); backend.has_value())
    {
      return backend->second->CreateFile(backend->first, overwriteExisting);
    }

    return false;
  }

  bool VirtualFileSystem::DeleteFile(const Path &path) noexcept
  {
    if (auto backend = GetBackend(path); backend.has_value())
    {
      return backend->second->DeleteFile(backend->first);
    }

    return false;
  }

  List<VFSFileEntry> VirtualFileSystem::SearchFiles(const Path &directory, FileSearchFlags flags) const
  {
    Set<Path> seen; // key by RELATIVE path string (virtual)
    List<VFSFileEntry> out;

    // We get back the path relative to the backend's root
    for (const auto &[relativeRoot, backend] : GetBackends(directory))
    {
      for (auto &entry : backend->SearchFiles(relativeRoot, flags))
      {
        if (seen.insert(entry.RelativePath.RelativePath(relativeRoot)).second)
        {
          out.emplace_back(std::move(entry)); // first in precedence order wins
        }
      }
    }
    return out;
  }

  List<Pair<Path, Ptr<IFileBackend>>> VirtualFileSystem::GetBackends(const Path &path) const noexcept
  {
    List<Pair<Path, Ptr<IFileBackend>>> result;
    for (const auto &[alias, backend] : _backends)
    {
      if (path.StartsWith(alias))
      {
        try
        {
          // strips the alias from the path to get path relative to the backend root
          result.emplace_back(path.RelativePath(alias), backend.get());
        }
        catch (const std::exception &ex)
        {
          KRYS_ERROR("Failed to get relative path for backend '{}': {}", alias.ToPlatformString(), ex.what());
          KRYS_DEBUG_BREAK();
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
