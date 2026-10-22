#include "Krystal.Booey/HTML/ColorInputType.hpp"
#include "Krystal.Booey/HTML/HTMLInputElement.hpp"
#include "Krystal.Core/Text/ASCII.hpp"
#include <string_view>

namespace krys::boo::html
{
  namespace
  {
    KRYS_NODISCARD bool IsValidSimpleColor(dom::DOMStringView string) noexcept
    {
      if (string.size() != 7)
      {
        return false;
      }
      if (string[0] != u8'#')
      {
        return false;
      }
      for (size_t i = 1; i < 7; ++i)
      {
        if (!text::IsASCIIHexDigit(string[i]))
        {
          return false;
        }
      }
      return true;
    }

    // https://html.spec.whatwg.org/#rules-for-parsing-simple-colour-values
    KRYS_NODISCARD Maybe<SRGBA<uint8>> ParseSimpleColorValue(dom::DOMStringView string) noexcept
    {
      if (!IsValidSimpleColor(string))
      {
        return std::nullopt;
      }
      auto hex = [&](size_t i)
      {
        return static_cast<uint8>(text::ToASCIIHexValue(string[i]) * 16
                                  + text::ToASCIIHexValue(string[i + 1]));
      };
      return SRGBA<uint8> {hex(1), hex(3), hex(5), 255};
    }

    /// @brief The serialisation of a simple colour: # and six lowercase hexadecimal digits.
    KRYS_NODISCARD dom::DOMString SerializeColorValue(const SRGBA<uint8> &color) noexcept
    {
      constexpr std::string_view hexDigits = "0123456789abcdef";
      dom::DOMString result(u8"#");
      auto resolved = ResolvedColour(color);
      for (uint8 component : {resolved.Red, resolved.Green, resolved.Blue})
      {
        result += static_cast<char8>(hexDigits[component >> 4]);
        result += static_cast<char8>(hexDigits[component & 0xF]);
      }
      return result;
    }
  }

  bool ColorInputType::SupportsRequired() const noexcept
  {
    return false;
  }

  Maybe<dom::DOMString> ColorInputType::FallbackValue() const noexcept
  {
    return SerializeColorValue({0, 0, 0, 255});
  }

  dom::DOMString ColorInputType::SanitizeValue(dom::DOMStringView proposedValue) const noexcept
  {
    auto color = ParseSimpleColorValue(proposedValue);

    if (!color)
    {
      return *FallbackValue();
    }

    return SerializeColorValue(*color);
  }

  SRGBA<uint8> ColorInputType::ValueAsColor() const noexcept
  {
    krys_debug_assert(Element());
    auto color = ParseSimpleColorValue(Element()->Value());
    krys_debug_assert(!!color);
    return color.value_or(SRGBA<uint8> {0, 0, 0, 255});
  }
}
