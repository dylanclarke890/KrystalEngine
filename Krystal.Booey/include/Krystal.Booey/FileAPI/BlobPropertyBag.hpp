#pragma once

#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/Types/String.hpp"

namespace krys::boo
{
  /// @brief How a blob's string parts keep their line endings.
  /// @see https://w3c.github.io/FileAPI/#enumdef-endingtype
  enum class EndingType : bool
  {
    /// @brief As they are.
    Transparent,
    /// @brief Converted to the platform's (CRLF on Windows, LF elsewhere).
    Native
  };

  /// @see https://w3c.github.io/FileAPI/#dfn-BlobPropertyBag
  struct BlobPropertyBag
  {
    utf8_string Type;
    EndingType Endings {EndingType::Transparent};
  };
}
