#include "Krystal.IO/Path.hpp"
#include <utility>

namespace Krys::IO
{
  Path::Path(const std::filesystem::path &path) : _path(path)
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

  Path::operator bool() const noexcept
  {
    return !_path.empty();
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
    auto a = _path.lexically_normal();
    auto b = other._path.lexically_normal();

    auto itA = a.begin();
    for (auto itB = b.begin(); itB != b.end(); ++itB, ++itA)
    {
      if (itA == a.end() || *itA != *itB)
        return false;
    }
    return true;
  }

  bool Path::IsRelative() const noexcept
  {
    return _path.is_relative();
  }

  bool Path::IsAbsolute() const noexcept
  {
    return _path.is_absolute();
  }

  Path Path::RelativePath(const Path &base) const
  {
    return Path(_path.lexically_relative(base._path));
  }

  bool Path::IsRelativeTo(const Path &other) const
  {
    return !_path.lexically_relative(other._path).empty();
  }

  Path Path::Normalise() const
  {
    return Path(_path.lexically_normal());
  }

  string Path::ToPlatformString() const
  {
    return _path.string();
  }

  string Path::ToString() const
  {
    return _path.generic_string();
  }
}
