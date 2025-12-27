#include "Krystal.IO/Backends/NativeFileBackend.hpp"
#include "Krystal.IO/Streams/NativeFileStream.hpp"
#include "Krystal.Lib/Core/DebugBreak.hpp"
#include "Krystal.Log/ILogger.hpp"
#include <filesystem>
#include <fstream>

namespace Krys::IO
{
  NativeFileBackend::NativeFileBackend(const Path &root) : _root(root)
  {
  }

  NativeFileBackend::NativeFileBackend(Path &&root) : _root(std::move(root))
  {
  }

  bool NativeFileBackend::Exists(const Path &path) const noexcept
  {
    try
    {
      return std::filesystem::exists((_root / path).Normalise().NativePath());
    }
    catch (const std::exception &ex)
    {
      KRYS_ERROR("Failed to check existence of '{}': {}", path.ToPlatformString(), ex.what());
      KRYS_DEBUG_BREAK();
      return false;
    }
  }

  bool NativeFileBackend::IsDirectory(const Path &path) const noexcept
  {
    try
    {
      return std::filesystem::is_directory((_root / path).Normalise().NativePath());
    }
    catch (const std::exception &ex)
    {
      KRYS_ERROR("Failed to check if '{}' is a directory: {}", path.ToPlatformString(), ex.what());
      KRYS_DEBUG_BREAK();
      return false;
    }
  }

  bool NativeFileBackend::IsFile(const Path &path) const noexcept
  {
    try
    {
      return std::filesystem::is_regular_file((_root / path).Normalise().NativePath());
    }
    catch (const std::exception &ex)
    {
      KRYS_ERROR("Failed to check if '{}' is a file: {}", path.ToPlatformString(), ex.what());
      KRYS_DEBUG_BREAK();
      return false;
    }
  }

  bool NativeFileBackend::CreateFile(const Path &path, bool overwriteExisting)
  {
    const std::filesystem::path fullPath = (_root / path).Normalise().NativePath();
    if (!overwriteExisting && std::filesystem::exists(fullPath))
    {
      return false;
    }

    std::filesystem::create_directories(fullPath.parent_path());
    std::ofstream(fullPath, std::ios::trunc).close(); // Create or overwrite the file

    return std::filesystem::exists(fullPath);
  }

  bool NativeFileBackend::DeleteFile(const Path &path)
  {
    const std::filesystem::path fullPath = (_root / path).Normalise().NativePath();
    if (std::filesystem::exists(fullPath) && std::filesystem::is_regular_file(fullPath))
    {
      return std::filesystem::remove(fullPath);
    }
    return false; // File does not exist or is not a regular file
  }

  List<VFSFileEntry> NativeFileBackend::SearchFiles(const Path &directory,
                                                    FileSearchFlags flags) const noexcept
  {
    try
    {
      const std::filesystem::path fullPath = (_root / directory).Normalise().NativePath();
      if (!std::filesystem::is_directory(fullPath))
      {
        return {};
      }

      if (!!(flags & FileSearchFlags::Recursive))
      {
        return SearchFiles<true>(fullPath, flags);
      }

      return SearchFiles<false>(fullPath, flags);
    }
    catch (const std::exception &ex)
    {
      KRYS_ERROR("Failed to search files in '{}': {}", directory.ToPlatformString(), ex.what());
      KRYS_DEBUG_BREAK();
      return {};
    }
  }

  Unique<IStreamReader> NativeFileBackend::GetReader(const Path &path, ReadFlags flags) const
  {
    if (Exists(path))
    {
      IStreamReader *reader = new NativeFileReader((_root / path).Normalise(), flags);
      return Unique<IStreamReader>(reader);
    }

    return nullptr;
  }

  Unique<IStreamWriter> NativeFileBackend::GetWriter(const Path &path, WriteFlags flags) const
  {
    IStreamWriter *writer = new NativeFileWriter((_root / path).Normalise(), flags);
    return Unique<IStreamWriter>(writer);
  }
}