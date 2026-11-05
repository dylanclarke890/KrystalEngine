#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo
{
  class SharedBuffer;

  /// @brief If @p buffer is a WOFF or WOFF 2 file, converts it to an sfnt (TrueType or OpenType) font.
  /// @return Whether @p buffer was converted. If the conversion fails, @p buffer becomes null.
  bool ConvertWOFFToSfntIfNecessary(RefPtr<SharedBuffer> &buffer) noexcept;
}
