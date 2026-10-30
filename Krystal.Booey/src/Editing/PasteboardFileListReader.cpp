#include "Krystal.Booey/Editing/PasteboardFileListReader.hpp"

namespace krys::boo
{
  void PasteboardFileListReader::ReadFilename(utf8_stringview filename) noexcept
  {
    Files.push_back(File::Create(filename));
  }

  void PasteboardFileListReader::ReadBuffer(utf8_stringview filename, utf8_stringview type,
                                            Ref<SharedBuffer> &&buffer) noexcept
  {
    Files.push_back(File::Create(*Blob::Create(krys::move(buffer), type), filename));
  }
}
