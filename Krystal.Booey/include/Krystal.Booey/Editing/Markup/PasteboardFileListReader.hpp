#pragma once

#include "Krystal.Booey/FileAPI/File.hpp"
#include "Krystal.Booey/Platform/Pasteboard.hpp"

namespace krys::boo
{
  /// @brief Reads the files on a pasteboard as `File`s: by their paths, or from their buffers.
  struct PasteboardFileListReader final : PasteboardFileReader
  {
    List<Ref<File>> Files;

    void ReadFilename(utf8_stringview filename) noexcept final;
    void ReadBuffer(utf8_stringview filename, utf8_stringview type,
                    Ref<SharedBuffer> &&buffer) noexcept final;
  };
}
