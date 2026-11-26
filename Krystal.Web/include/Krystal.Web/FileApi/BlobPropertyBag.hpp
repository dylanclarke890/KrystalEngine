#pragma once

#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/Strings/String.hpp"

namespace krys::web
{
  /// @brief How a blob's string parts keep their line endings.
  /// @see https://w3c.github.io/FileApi/#enumdef-endingtype
  enum class EndingType : bool
  {
    /// @brief As they are.
    Transparent,
    /// @brief Converted to the platform's (CRLF on Windows, LF elsewhere).
    Native,
  };

  /// @see https://w3c.github.io/FileApi/#dfn-BlobPropertyBag
  struct BlobPropertyBag
  {
    utf8_string Type;
    EndingType Endings {EndingType::Transparent};
  };
}
