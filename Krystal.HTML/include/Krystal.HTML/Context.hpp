#pragma once

#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Text/Codecs/CodecRegistry.hpp"

namespace Krys::HTML
{
  struct Context
  {
    /// @brief Provides access to all common codecs for decoding HTML/CSS text resources.
    Krys::Text::CodecRegistry DecoderRegistry;
  };

  KRYS_NODISCARD Context CreateContext() noexcept;
}