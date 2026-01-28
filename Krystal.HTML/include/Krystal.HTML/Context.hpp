#pragma once

#include "Krystal.HTML/Loader/ContentType.hpp"
#include "Krystal.HTML/Loader/HTMLEncodingSniffer.hpp"
#include "Krystal.HTML/Performance.hpp"
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
    Text::CodecRegistry DecoderRegistry;

    /// @brief The performance object associated with this context.
    Performance Performance;
  };

  KRYS_NODISCARD Context CreateContext() noexcept;
}