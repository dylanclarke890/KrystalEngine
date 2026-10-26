#include "Krystal.Booey/FileAPI/FileList.hpp"

namespace krys::boo
{
  File *FileList::Item(uint32 index) const noexcept
  {
    if (index >= _files.size())
    {
      return nullptr;
    }
    return _files[index].get();
  }

  List<utf8_string> FileList::Paths() const noexcept
  {
    List<utf8_string> paths;
    paths.reserve(_files.size());
    for (auto &file : _files)
    {
      paths.emplace_back(file->Path());
    }
    return paths;
  }
}
