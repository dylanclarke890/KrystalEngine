#pragma once

#include "Krystal.Core/Enum.hpp"
#include "Krystal.Core/Numeric.hpp"

namespace krys::boo::html
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

KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(krys::boo::html::EncodingSource, 4u)
