#pragma once

#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Mixins/NonCopyMovable.hpp"
#include "Krystal.Lib/Types/Array.hpp"
#include "Krystal.Lib/Types/Numeric.hpp"
#include "Krystal.Lib/Types/Span.hpp"
#include "Krystal.Text/Encodings/Encoding.hpp"

namespace Krys::HTML::DOM
{
  enum class BOMEncoding
  {
    None,
    UTF8,
    UTF16BE,
    UTF16LE,
  };
  
  struct EncodingSniffer : NonCopyMovable<EncodingSniffer>
  {
    KRYS_NODISCARD static BOMEncoding GetBOMEncoding(Span<const byte> bytes) noexcept
    {
      using namespace std::string_view_literals;

      Encoding *encoding = nullptr;

      if (SniffBOM(bytes, Span<const byte> {u8"\xEF\xBB\xBF"sv}))
      {
        return BOMEncoding::UTF8;
      }
      else if (SniffBOM(bytes, Span<const byte> {u8"\xFE\xFF"sv}))
      {
        return BOMEncoding::UTF16BE;
      }
      else if (SniffBOM(bytes, Span<const byte> {u8"\xFF\xFE"sv}))
      {
        return BOMEncoding::UTF16LE;
      }
    }

    private:
    constexpr static bool SniffBOM(Span<const byte> bytes, Span<const byte> bom) noexcept
    {
    }
  };
}