#include "Krystal.IO/Backends/NativeFileBackend.hpp"
#include "Krystal.IO/Streams/NativeFileStream.hpp"
#include <filesystem>
#include <fstream>

namespace Krys::IO
{
  NativeFileBackend::NativeFileBackend(const Path &root) noexcept : _root(root)
  {
  }

  bool NativeFileBackend::Exists(const Path &path) const noexcept
  {
    return std::filesystem::exists((_root / path).LexicallyNormal().ToString());
  }

  bool NativeFileBackend::IsDirectory(const Path &path) const noexcept
  {
    return std::filesystem::is_directory((_root / path).LexicallyNormal().ToString());
  }

  bool NativeFileBackend::IsFile(const Path &path) const noexcept
  {
    return std::filesystem::is_regular_file((_root / path).LexicallyNormal().ToString());
  }

  bool NativeFileBackend::CreateFile(const Path &path, bool overwriteExisting) noexcept
  {
    std::filesystem::path fullPath = (_root / path).LexicallyNormal().ToString();
    if (!overwriteExisting && std::filesystem::exists(fullPath))
    {
      return false;
    }

    std::filesystem::create_directories(fullPath.parent_path());
    std::ofstream(fullPath, std::ios::trunc).close(); // Create or overwrite the file

    return std::filesystem::exists(fullPath);
  }

  bool NativeFileBackend::DeleteFile(const Path &path) noexcept
  {
    std::filesystem::path fullPath = (_root / path).ToString();
    if (std::filesystem::exists(fullPath) && std::filesystem::is_regular_file(fullPath))
    {
      return std::filesystem::remove(fullPath);
    }
    return false; // File does not exist or is not a regular file
  }

  List<VirtualDirectoryEntry> NativeFileBackend::GetDirectoryEntries(const Path &directory,
                                                                     bool recursive) const noexcept
  {
    List<VirtualDirectoryEntry> entries;
    std::filesystem::path fullPath = (_root / directory).ToString();

    if (!std::filesystem::is_directory(fullPath))
    {
      return entries; // Not a directory, return empty list
    }

    Path root(_root);
    for (const auto &entry : std::filesystem::directory_iterator(fullPath))
    {
      if (!entry.is_directory() && !entry.is_regular_file())
      {
        continue;
      }

      VirtualDirectoryEntry directoryEntry {Path(entry.path()).RelativePath(root),
                                            entry.is_regular_file() ? entry.file_size() : 0,
                                            entry.is_directory()};
      if (entry.is_directory() && recursive)
      {
        List<VirtualDirectoryEntry> subEntries =
          GetDirectoryEntries(directoryEntry.Path.RelativePath(root), true);
        entries.insert(entries.end(), subEntries.begin(), subEntries.end());
      }
      else
      {
        entries.push_back(directoryEntry);
      }
    }

    return entries;
  }

  Unique<IStreamReader> NativeFileBackend::GetReader(const Path &path) const noexcept
  {
    if (Exists(path))
    {
      return Unique<IStreamReader>(static_cast<IStreamReader *>(new NativeFileReader(_root / path)));
    }

    return nullptr; // Cannot create a reader for a non-existing or non-regular file
  }

  Unique<IStreamWriter> NativeFileBackend::GetWriter(const Path &path) const noexcept
  {
    return Unique<IStreamWriter>(static_cast<IStreamWriter *>(new NativeFileWriter(_root / path)));
  }
}