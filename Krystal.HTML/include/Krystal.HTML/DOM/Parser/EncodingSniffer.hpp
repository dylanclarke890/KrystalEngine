#pragma once

#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Mixins/NonCopyMovable.hpp"
#include "Krystal.Lib/Types/Array.hpp"
#include "Krystal.Lib/Types/Map.hpp"
#include "Krystal.Lib/Types/Numeric.hpp"
#include "Krystal.Lib/Types/Span.hpp"
#include "Krystal.Text/Encodings/ByteOrderMark.hpp"
#include "Krystal.Text/Encodings/EncodingRegistry.hpp"
#include "Krystal.Text/Unicode.hpp"

namespace Krys::HTML::DOM
{
  struct EncodingSniffer : NonCopyMovable<EncodingSniffer>
  {
    EncodingSniffer() = delete;
    ~EncodingSniffer() = delete;

    using EncodingLabelMap = Map<utf8_string, List<utf8_string>>;

    KRYS_NODISCARD static utf8_string Detect(Span<const byte> bytes, const utf8_string &transportEncoding,
                                             const utf8_string &fallbackEncoding) noexcept
    {
      utf8_string label = GetBOMEncoding(bytes);
      if (!label.empty())
      {
        return label;
      }
    }

  private:
    KRYS_NODISCARD constexpr static utf8_string GetBOMEncoding(Span<const byte> bytes) noexcept
    {
      auto bom = ByteOrderMarkScanner::Detect(bytes);
      switch (bom)
      {
        case ByteOrderMark::UTF8:
        case ByteOrderMark::UTF16BE:
        case ByteOrderMark::UTF16LE: return ByteOrderMarkScanner::GetEncodingName(bom);
      }

      return u8"";
    }

    KRYS_NODISCARD constexpr static bool IsWhitespace(UnicodeCodepoint ch) noexcept
    {
      return ch == UnicodeCharacter::Space || ch == UnicodeCharacter::Tab || ch == UnicodeCharacter::LF
             || ch == UnicodeCharacter::CR;
    }
  };
}