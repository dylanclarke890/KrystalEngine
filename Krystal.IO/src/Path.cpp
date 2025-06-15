#include "Krystal.IO/Path.hpp"

#include "Krystal.Core/Core.hpp"

#include <filesystem>
#include <utility>

namespace Krys::IO
{
  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init)
  Path::Path(std::filesystem::path path) noexcept : _path(path.lexically_normal())
  {
  }

  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init)
  Path::Path(const Path &other) noexcept : _path(other._path)
  {
  }

  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init)
  Path::Path(Path &&other) noexcept : _path(std::move(other._path))
  {
  }

  Path &Path::operator=(const Path &other)
  {
    _path = other._path;
    return *this;
  }

  Path &Path::operator=(Path &&other) noexcept
  {
    _path = std::move(other._path);
    return *this;
  }

  NO_DISCARD Path Path::operator/(const Path &other) const noexcept
  {
    return Path(_path / other._path);
  }

  Path &Path::operator/=(const Path &other) noexcept
  {
    *this = *this / other;
    return *this;
  }

  NO_DISCARD bool Path::operator==(const Path &other) const noexcept
  {
    return _path == other._path;
  }

  NO_DISCARD bool Path::operator!=(const Path &other) const noexcept
  {
    return _path != other._path;
  }

  NO_DISCARD Path Path::ParentPath() const noexcept
  {
    return Path(_path.parent_path());
  }

  NO_DISCARD bool Path::HasFilename() const noexcept
  {
    return _path.has_filename();
  }

  NO_DISCARD Path Path::Filename() const noexcept
  {
    return Path(_path.filename());
  }

  NO_DISCARD bool Path::HasExtension() const noexcept
  {
    return _path.has_extension();
  }

  NO_DISCARD Path Path::Extension() const noexcept
  {
    return Path(_path.extension());
  }

  NO_DISCARD bool Path::StartsWith(const Path &other) const noexcept
  {
    return _path.string().starts_with(other._path.string());
  }

  NO_DISCARD bool Path::IsRelative() const noexcept
  {
    return _path.is_relative();
  }

  NO_DISCARD bool Path::IsRelativeTo(const Path &other) const noexcept
  {
    return !_path.lexically_relative(other._path).empty();
  }

  NO_DISCARD Path Path::RelativePath(const Path &base) const noexcept
  {
    if (!StartsWith(base))
    {
      return *this; // return the full path if base is not a parent
    }

    return Path(_path.lexically_relative(base._path).string());
  }

  NO_DISCARD Path Path::LexicallyNormal() const noexcept
  {
    return Path(_path.lexically_normal());
  }

  NO_DISCARD string Path::ToString() const noexcept
  {
    return _path.string();
  }

  Path::operator bool() const noexcept
  {
    return !_path.empty();
  }

  NO_DISCARD size_t Path::Length() const noexcept
  {
    return _path.string().size();
  }
}
