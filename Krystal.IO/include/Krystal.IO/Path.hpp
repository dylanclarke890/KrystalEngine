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
    explicit Path(const std::filesystem::path &path);

    ~Path() noexcept = default;

    Path(const Path &other) = default;

    Path(Path &&other) noexcept;

    Path &operator=(const Path &other) = default;

    Path &operator=(Path &&other) noexcept;

    NO_DISCARD Path operator/(const Path &other) const;

    Path &operator/=(const Path &other);

    NO_DISCARD bool operator==(const Path &other) const noexcept;

    NO_DISCARD bool operator!=(const Path &other) const noexcept;

    NO_DISCARD explicit operator bool() const noexcept;

    NO_DISCARD Path ParentPath() const;

    NO_DISCARD bool HasFilename() const noexcept;

    NO_DISCARD Path Filename() const;

    NO_DISCARD bool HasExtension() const noexcept;

    NO_DISCARD Path Extension() const;

    NO_DISCARD bool StartsWith(const Path &other) const;

    NO_DISCARD bool IsRelative() const noexcept;

    NO_DISCARD bool IsRelativeTo(const Path &other) const;

    NO_DISCARD Path RelativePath(const Path &base) const;

    NO_DISCARD Path LexicallyNormal() const;

    NO_DISCARD string ToString() const;

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