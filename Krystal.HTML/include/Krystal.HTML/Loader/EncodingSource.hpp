#pragma once

#include "Krystal.Lib/Core/Enum.hpp"
#include "Krystal.Lib/Types/Numeric.hpp"

namespace Krys::HTML
{
  enum class EncodingSource : uint8
  {
    /// @brief Default encoding.
    Default,

    /// @brief Encoding specified via BOM.
    BOM,

    /// @brief Encoding specified via HTTP transport layer.
    Transport,

    /// @brief Encoding specified via &lt;meta&gt; tag in HTML.
    MetaTag,
  };
}

namespace Krys
{
  template <>
  constexpr underlying_t<HTML::EncodingSource> OrdinalCount<HTML::EncodingSource> = 4;
}