#pragma once

#include "Krystal.Lib/Core/Compiler.hpp"
#include "Krystal.Lib/String/String.hpp"
#include "Krystal.Lib/Types/Numeric.hpp"
#include <filesystem>

namespace Krys::IO
{
  /// @brief Represents a file system path in a platform-independent manner.
  class Path
  {
    std::filesystem::path _path;

  public:
    Path() = default;

    explicit Path(const std::filesystem::path &path);

    ~Path() noexcept = default;

    Path(const Path &other) = default;

    Path &operator=(const Path &other) = default;

    Path(Path &&other) noexcept;

    Path &operator=(Path &&other) noexcept;

    /// @brief Combines two paths using the appropriate platform-specific separator.
    KRYS_NODISCARD Path operator/(const Path &other) const;

    /// @brief Appends another path to this path using the appropriate platform-specific separator.
    Path &operator/=(const Path &other);

    /// @brief Compares two paths for equality.
    KRYS_NODISCARD bool operator==(const Path &other) const noexcept;

    /// @brief Compares two paths for inequality.
    KRYS_NODISCARD bool operator!=(const Path &other) const noexcept;

    /// @brief Checks if the path is non-empty.
    KRYS_NODISCARD explicit operator bool() const noexcept;

    /// @brief Returns the parent directory of the path.
    KRYS_NODISCARD Path ParentPath() const;

    /// @brief Checks if the path has a filename component.
    KRYS_NODISCARD bool HasFilename() const noexcept;

    /// @brief Returns the filename component of the path.
    KRYS_NODISCARD Path Filename() const;

    /// @brief Checks if the path has a file extension.
    KRYS_NODISCARD bool HasExtension() const noexcept;

    /// @brief Returns the file extension of the path, including the leading dot.
    KRYS_NODISCARD Path Extension() const;

    /// @brief Checks if this path starts with another path. This is a lexically normalized comparison of the
    /// components of the paths, not a simple string prefix check.
    KRYS_NODISCARD bool StartsWith(const Path &other) const;

    /// @brief Checks if the path is relative.
    KRYS_NODISCARD bool IsRelative() const noexcept;

    /// @brief Checks if the path is absolute.
    KRYS_NODISCARD bool IsAbsolute() const noexcept;

    /// @brief Returns the path relative to the specified base path.
    KRYS_NODISCARD Path RelativePath(const Path &base) const;

    /// @brief Checks if this path is relative to another path.
    KRYS_NODISCARD bool IsRelativeTo(const Path &other) const;

    /// @brief Returns a normalized version of the path, resolving any '.' or '..' components and removing
    /// redundant separators.
    KRYS_NODISCARD Path Normalise() const;

    /// @brief Converts the path to a string formatted for the current platform.
    KRYS_NODISCARD string ToPlatformString() const;

    /// @brief Converts the path to a platform-independent string (using '/' as separator).
    KRYS_NODISCARD string ToString() const;

    KRYS_NODISCARD const std::filesystem::path &NativePath() const noexcept
    {
      return _path;
    }
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