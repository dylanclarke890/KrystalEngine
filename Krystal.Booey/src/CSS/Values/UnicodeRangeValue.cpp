#include "Krystal.Booey/CSS/Values/UnicodeRangeValue.hpp"
#include "Krystal.Core/Assert.hpp"
#include <charconv>

namespace krys::boo::css
{
  namespace
  {
    // Lowercase hex digits, no leading zeros.
    void AppendLowercaseHex(CSSOMString &result, char32 value) noexcept
    {
      char buffer[8];
      auto [end, error] = std::to_chars(buffer, buffer + sizeof(buffer), static_cast<uint32>(value), 16);
      krys_debug_assert(error == std::errc {});
      for (const char *it = buffer; it != end; ++it)
      {
        result.push_back(static_cast<char8_t>(*it));
      }
    }
  }

  CSSOMString UnicodeRangeValue::CustomCSSText(const SerialisationContext &) const noexcept
  {
    CSSOMString result {u8"U+"};
    AppendLowercaseHex(result, _from);
    if (_from == _to)
    {
      return result;
    }

    result.push_back(u8'-');
    AppendLowercaseHex(result, _to);
    return result;
  }

  bool UnicodeRangeValue::Equals(const UnicodeRangeValue &other) const noexcept
  {
    return _from == other._from && _to == other._to;
  }
}
