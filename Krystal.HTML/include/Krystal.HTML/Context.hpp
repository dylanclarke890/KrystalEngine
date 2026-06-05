#pragma once

#include "Krystal.HTML/Parser/Loader/ContentType.hpp"
#include "Krystal.HTML/Parser/Loader/HTMLEncodingSniffer.hpp"
#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/String/String.hpp"
#include "Krystal.Lib/Types/Numeric.hpp"
#include "Krystal.Text/ASCII.hpp"
#include "Krystal.Text/Codecs/CodecRegistry.hpp"
#include "Krystal.Text/Codecs/ICodec.hpp"
#include "Krystal.Text/Decode/Decode.hpp"

namespace Krys::HTML
{
  struct Context
  {
    /// @brief Provides access to all common codecs for decoding HTML/CSS text resources.
    Krys::Text::CodecRegistry DecoderRegistry;
  };

  KRYS_NODISCARD Context CreateContext() noexcept;
}