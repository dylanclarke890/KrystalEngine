#include "Krystal.IO/Path.hpp"
#include <filesystem>
#include <utility>

namespace Krys::IO
{
  Path::Path(const std::filesystem::path &path) : _path(path.lexically_normal())
  {
  }

  Path::Path(Path &&other) noexcept : _path(std::move(other._path))
  {
  }

  Path &Path::operator=(Path &&other) noexcept
  {
    _path = std::move(other._path);
    return *this;
  }

  Path Path::operator/(const Path &other) const
  {
    return Path(_path / other._path);
  }

  Path &Path::operator/=(const Path &other)
  {
    *this = *this / other;
    return *this;
  }

  bool Path::operator==(const Path &other) const noexcept
  {
    return _path == other._path;
  }

  bool Path::operator!=(const Path &other) const noexcept
  {
    return _path != other._path;
  }

  Path Path::ParentPath() const
  {
    return Path(_path.parent_path());
  }

  bool Path::HasFilename() const noexcept
  {
    return _path.has_filename();
  }

  Path Path::Filename() const
  {
    return Path(_path.filename());
  }

  bool Path::HasExtension() const noexcept
  {
    return _path.has_extension();
  }

  Path Path::Extension() const
  {
    return Path(_path.extension());
  }

  bool Path::StartsWith(const Path &other) const
  {
    return _path.string().starts_with(other._path.string());
  }

  bool Path::IsRelative() const noexcept
  {
    return _path.is_relative();
  }

  bool Path::IsRelativeTo(const Path &other) const
  {
    return !_path.lexically_relative(other._path).empty();
  }

  Path Path::RelativePath(const Path &base) const
  {
    if (!StartsWith(base))
    {
      return *this; // return the full path if base is not a parent
    }

    return Path(_path.lexically_relative(base._path).string());
  }

  Path Path::LexicallyNormal() const
  {
    return Path(_path.lexically_normal());
  }

  string Path::ToString() const
  {
    return _path.string();
  }

  Path::operator bool() const noexcept
  {
    return !_path.empty();
  }

  size_t Path::Length() const noexcept
  {
    return _path.string().size();
  }
}
