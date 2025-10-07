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

    Path &operator=(const Path &other) = default;

    Path(Path &&other) noexcept;

    Path &operator=(Path &&other) noexcept;

    /// @brief Combines two paths using the appropriate platform-specific separator.
    NO_DISCARD Path operator/(const Path &other) const;

    /// @brief Appends another path to this path using the appropriate platform-specific separator.
    Path &operator/=(const Path &other);

    /// @brief Compares two paths for equality.
    NO_DISCARD bool operator==(const Path &other) const noexcept;

    /// @brief Compares two paths for inequality.
    NO_DISCARD bool operator!=(const Path &other) const noexcept;

    /// @brief Checks if the path is non-empty.
    NO_DISCARD explicit operator bool() const noexcept;

    /// @brief Returns the parent directory of the path.
    NO_DISCARD Path ParentPath() const;

    /// @brief Checks if the path has a filename component.
    NO_DISCARD bool HasFilename() const noexcept;

    /// @brief Returns the filename component of the path.
    NO_DISCARD Path Filename() const;

    /// @brief Checks if the path has a file extension.
    NO_DISCARD bool HasExtension() const noexcept;

    /// @brief Returns the file extension of the path, including the leading dot.
    NO_DISCARD Path Extension() const;

    NO_DISCARD bool StartsWith(const Path &other) const;

    /// @brief Checks if the path is relative.
    NO_DISCARD bool IsRelative() const noexcept;

    /// @brief Checks if the path is absolute.
    NO_DISCARD bool IsAbsolute() const noexcept;

    /// @brief Returns the path relative to the specified base path.
    NO_DISCARD Path RelativePath(const Path &base) const;

    /// @brief Checks if this path is relative to another path.
    NO_DISCARD bool IsRelativeTo(const Path &other) const;

    /// @brief Returns a normalized version of the path, resolving any '.' or '..' components and removing
    /// redundant separators.
    NO_DISCARD Path Normalise() const;

    /// @brief Converts the path to a string formatted for the current platform.
    NO_DISCARD string ToPlatformString() const;

    /// @brief Converts the path to a platform-independent string (using '/' as separator).
    NO_DISCARD string ToString() const;
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