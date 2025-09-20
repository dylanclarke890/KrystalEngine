#pragma once

#include "Krystal.Lib/Attributes.hpp"
#include "Krystal.Lib/String.hpp"
#include "Krystal.Lib/Types.hpp"
#include <filesystem>

namespace Krys::IO
{
  /// @brief Represents a file system path in a platform-independent manner.
  class Path
  {
    std::filesystem::path _path;

  public:
    explicit Path(std::filesystem::path path) noexcept;

    ~Path() noexcept = default;

    Path(const Path &other) noexcept;

    Path(Path &&other) noexcept;

    Path &operator=(const Path &other);

    Path &operator=(Path &&other) noexcept;

    NO_DISCARD Path operator/(const Path &other) const noexcept;

    Path &operator/=(const Path &other) noexcept;

    NO_DISCARD bool operator==(const Path &other) const noexcept;

    NO_DISCARD bool operator!=(const Path &other) const noexcept;

    NO_DISCARD explicit operator bool() const noexcept;

    NO_DISCARD Path ParentPath() const noexcept;

    NO_DISCARD bool HasFilename() const noexcept;

    NO_DISCARD Path Filename() const noexcept;

    NO_DISCARD bool HasExtension() const noexcept;

    NO_DISCARD Path Extension() const noexcept;

    NO_DISCARD bool StartsWith(const Path &other) const noexcept;

    NO_DISCARD bool IsRelative() const noexcept;

    NO_DISCARD bool IsRelativeTo(const Path &other) const noexcept;

    NO_DISCARD Path RelativePath(const Path &base) const noexcept;

    NO_DISCARD Path LexicallyNormal() const noexcept;

    NO_DISCARD string ToString() const noexcept;

    NO_DISCARD size_t Length() const noexcept;
  };
}

namespace std
{
  template <>
  struct hash<Krys::IO::Path>
  {
    size_t operator()(const Krys::IO::Path &path) const noexcept
    {
      return std::hash<Krys::string>()(path.ToString());
    }
  };
}