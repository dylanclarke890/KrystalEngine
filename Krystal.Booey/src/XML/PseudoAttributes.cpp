#include "Krystal.Booey/XML/PseudoAttributes.hpp"
#include "Krystal.Booey/DOM/Algorithms/NameValidation.hpp"
#include "Krystal.Core/Text/ASCII.hpp"
#include "Krystal.Core/Text/Encodings/UTF.hpp"
#include <algorithm>

namespace krys::boo::xml
{
  namespace
  {
    /// @see https://www.w3.org/TR/xml/#NT-S
    KRYS_NODISCARD constexpr bool IsPseudoAttributeSpace(char8 ch) noexcept
    {
      return ch == u8' ' || ch == u8'\t' || ch == u8'\r' || ch == u8'\n';
    }

    /// @see https://www.w3.org/TR/xml/#NT-Char
    KRYS_NODISCARD constexpr bool IsLegalXMLChar(char32 ch) noexcept
    {
      return ch == 0x9 || ch == 0xA || ch == 0xD || (ch >= 0x20 && ch <= 0xD7FF)
             || (ch >= 0xE000 && ch <= 0xFFFD) || (ch >= 0x10000 && ch <= 0x10FFFF);
    }

    /// @brief Reads a character reference (`&#38;`, `&#x26;`) or a predefined entity reference (`&amp;`) at
    /// the start of @p rest, appending what it represents to @p value; false if there's neither, or the
    /// character isn't legal.
    KRYS_NODISCARD bool ConsumePseudoAttributeReference(dom::DOMStringView &rest,
                                                        dom::DOMString &value) noexcept
    {
      static constexpr std::pair<dom::DOMStringView, char8> PredefinedEntities[] {
        {u8"&amp;", u8'&'}, {u8"&lt;", u8'<'}, {u8"&gt;", u8'>'}, {u8"&quot;", u8'"'}, {u8"&apos;", u8'\''}};
      for (auto [entity, character] : PredefinedEntities)
      {
        if (rest.starts_with(entity))
        {
          value += character;
          rest.remove_prefix(entity.size());
          return true;
        }
      }

      // CharRef ::= '&#' [0-9]+ ';' | '&#x' [0-9a-fA-F]+ ';'
      if (!rest.starts_with(u8"&#"))
      {
        return false;
      }
      bool hex = rest.size() > 2 && rest[2] == u8'x';
      size_t position = hex ? 3uz : 2uz;
      char32 codePoint = 0;
      size_t digits = 0;
      for (; position < rest.size() && rest[position] != u8';'; ++position, ++digits)
      {
        auto ch = rest[position];
        if (hex ? !text::IsASCIIHexDigit(ch) : !text::IsASCIIDigit(ch))
        {
          return false;
        }
        codePoint = codePoint * (hex ? 16 : 10) + static_cast<char32>(text::ToASCIIHexValue(ch));
        // Anything past the last code point is illegal; stop before it overflows.
        if (codePoint > 0x10FFFF)
        {
          return false;
        }
      }
      if (digits == 0 || position == rest.size() || !IsLegalXMLChar(codePoint))
      {
        return false;
      }

      auto encoded = text::UTF8::EncodeOne(codePoint);
      value.append(encoded.Output.data(), encoded.CodeUnitsProduced);
      rest.remove_prefix(position + 1);
      return true;
    }
  }

  Maybe<List<PseudoAttribute>> ParsePseudoAttributes(dom::DOMStringView string) noexcept
  {
    // PseudoAtts ::= PseudoAtt? (S PseudoAtt)* S?
    List<PseudoAttribute> result;
    auto rest = string;
    while (true)
    {
      bool hadSpace = false;
      while (!rest.empty() && IsPseudoAttributeSpace(rest.front()))
      {
        rest.remove_prefix(1);
        hadSpace = true;
      }
      if (rest.empty())
      {
        break;
      }
      // Pseudo-attributes after the first are separated by spaces.
      if (!result.empty() && !hadSpace)
      {
        return null;
      }

      // PseudoAtt ::= Name S? "=" S? PseudoAttValue
      auto nameEnd =
        std::ranges::find_if(rest, [](char8 ch) { return IsPseudoAttributeSpace(ch) || ch == u8'='; });
      dom::DOMStringView name(rest.begin(), nameEnd);
      if (!dom::NameValidation::IsValidXMLName(name))
      {
        return null;
      }
      rest.remove_prefix(name.size());
      while (!rest.empty() && IsPseudoAttributeSpace(rest.front()))
      {
        rest.remove_prefix(1);
      }
      if (rest.empty() || rest.front() != u8'=')
      {
        return null;
      }
      rest.remove_prefix(1);
      while (!rest.empty() && IsPseudoAttributeSpace(rest.front()))
      {
        rest.remove_prefix(1);
      }

      // PseudoAttValue ::= '"' ([^"<&] | CharRef | PredefEntityRef)* '"'
      //                  | "'" ([^'<&] | CharRef | PredefEntityRef)* "'"
      // The quotes are removed and the references replaced by the characters they represent.
      if (rest.empty() || (rest.front() != u8'"' && rest.front() != u8'\''))
      {
        return null;
      }
      auto quote = rest.front();
      rest.remove_prefix(1);
      dom::DOMString value;
      while (true)
      {
        if (rest.empty() || rest.front() == u8'<')
        {
          return null;
        }
        if (rest.front() == quote)
        {
          rest.remove_prefix(1);
          break;
        }
        if (rest.front() == u8'&')
        {
          if (!ConsumePseudoAttributeReference(rest, value))
          {
            return null;
          }
          continue;
        }
        value += rest.front();
        rest.remove_prefix(1);
      }

      // The parsing result is an error if there are more than one pseudo-attribute with the same name.
      if (std::ranges::any_of(result,
                              [&](const PseudoAttribute &attribute) { return attribute.Name == name; }))
      {
        return null;
      }
      result.push_back({dom::DOMString(name), krys::move(value)});
    }

    return result;
  }
}
