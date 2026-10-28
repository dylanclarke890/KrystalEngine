#include "Krystal.Booey/MathML/MathMLPresentationElement.hpp"
#include "Krystal.Booey/Infra/StringAlgorithms.hpp"
#include "Krystal.Booey/Rendering/MathML/RenderMathMLBlock.hpp"
#include "Krystal.Core/Text/ASCII.hpp"
#include <charconv>
#include <cmath>

namespace krys::boo::mathml
{
  MathMLPresentationElement::MathMLPresentationElement(dom::Document &document,
                                                       MathMLElementInterface interface,
                                                       dom::NodeFlags nodeFlags) noexcept
      : MathMLElement(document, interface, nodeFlags)
  {
  }

  RenderPtr<RenderElement> MathMLPresentationElement::CreateElementRenderer(RenderStyle &&style) noexcept
  {
    if (HasTagName(html::TagName::mtable))
    {
      return CreateRenderer<RenderMathMLTable>(*this, krys::move(style));
    }

    return MathMLElement::CreateElementRenderer(krys::move(style));
  }

  MathMLElement::BooleanValue
    MathMLPresentationElement::CachedBooleanAttribute(const dom::QualifiedName &name,
                                                      Maybe<BooleanValue> &attribute) const noexcept
  {
    if (attribute)
    {
      return *attribute;
    }

    // In MathML Core, attribute values are ASCII case-insensitive.
    // https://w3c.github.io/mathml-core/#dfn-boolean
    auto value = AttributeValue(name).value_or(dom::DOMString {});
    if (infra::StringAlgorithms::ASCIICaseInsensitiveMatch(value, u8"true"))
    {
      attribute = BooleanValue::True;
    }
    else if (infra::StringAlgorithms::ASCIICaseInsensitiveMatch(value, u8"false"))
    {
      attribute = BooleanValue::False;
    }
    else
    {
      attribute = BooleanValue::Default;
    }

    return *attribute;
  }

  MathMLElement::Length MathMLPresentationElement::ParseNumberAndUnit(dom::DOMStringView string) noexcept
  {
    LengthType lengthType = LengthType::UnitLess;
    size_t stringLength = string.size();
    char8 lastChar = string[stringLength - 1];
    if (lastChar == u8'%')
    {
      lengthType = LengthType::Percentage;
      stringLength--;
    }
    else if (stringLength >= 2)
    {
      char8 penultimateChar = string[stringLength - 2];
      if (penultimateChar == u8'c' && lastChar == u8'm')
      {
        lengthType = LengthType::Cm;
      }
      if (penultimateChar == u8'e' && lastChar == u8'm')
      {
        lengthType = LengthType::Em;
      }
      else if (penultimateChar == u8'e' && lastChar == u8'x')
      {
        lengthType = LengthType::Ex;
      }
      else if (penultimateChar == u8'i' && lastChar == u8'n')
      {
        lengthType = LengthType::In;
      }
      else if (penultimateChar == u8'm' && lastChar == u8'm')
      {
        lengthType = LengthType::Mm;
      }
      else if (penultimateChar == u8'p' && lastChar == u8'c')
      {
        lengthType = LengthType::Pc;
      }
      else if (penultimateChar == u8'p' && lastChar == u8't')
      {
        lengthType = LengthType::Pt;
      }
      else if (penultimateChar == u8'p' && lastChar == u8'x')
      {
        lengthType = LengthType::Px;
      }

      if (lengthType != LengthType::UnitLess)
      {
        stringLength -= 2;
      }
    }

    // The number takes the whole of what's left, after any leading whitespace.
    auto number = string.substr(0, stringLength);
    while (!number.empty() && text::IsUnicodeCompatibleASCIIWhitespace(number.front()))
    {
      number.remove_prefix(1);
    }
    float lengthValue {};
    const auto *first = reinterpret_cast<const char *>(number.data());
    const auto *last = first + number.size();
    auto result = std::from_chars(first, last, lengthValue);
    if (number.empty() || result.ec != std::errc {} || result.ptr != last || !std::isfinite(lengthValue))
    {
      return Length {};
    }

    if (lengthType == LengthType::UnitLess && string != u8"0")
    {
      return Length {};
    }

    return Length {.Type = lengthType, .Value = lengthValue};
  }

  MathMLElement::Length MathMLPresentationElement::ParseMathMLLength(dom::DOMStringView string) noexcept
  {
    // The regular expression from the MathML Relax NG schema is as follows:
    //
    //   pattern =
    //   '\s*((-?[0-9]*([0-9]\.?|\.[0-9])[0-9]*(e[mx]|in|cm|mm|p[xtc]|%)?)|(negative)?((very){0,2}thi(n|ck)|medium)mathspace)\s*'
    //
    // We do not perform a strict verification of the syntax of whitespaces and number.
    // Instead, we just use isASCIIWhitespace and toFloat to parse these parts.

    // We first skip whitespace from both ends of the string.
    auto trimmedLength = string;
    while (!trimmedLength.empty() && text::IsASCIIWhitespaceWithoutFF(trimmedLength.front()))
    {
      trimmedLength.remove_prefix(1);
    }
    while (!trimmedLength.empty() && text::IsASCIIWhitespaceWithoutFF(trimmedLength.back()))
    {
      trimmedLength.remove_suffix(1);
    }

    if (trimmedLength.empty())
    {
      return Length {};
    }

    // We consider the most typical case: a number followed by an optional unit.
    char8 firstChar = trimmedLength[0];
    if (text::IsASCIIDigit(firstChar) || firstChar == u8'-' || firstChar == u8'.')
    {
      return ParseNumberAndUnit(trimmedLength);
    }

    return Length {};
  }

  const MathMLElement::Length &
    MathMLPresentationElement::CachedMathMLLength(const dom::QualifiedName &name,
                                                  Maybe<Length> &length) const noexcept
  {
    if (!length)
    {
      length = ParseMathMLLength(AttributeValue(name).value_or(dom::DOMString {}));
    }
    return *length;
  }
}
