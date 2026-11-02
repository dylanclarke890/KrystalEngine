#include "Krystal.Booey/HTML/HTMLFontElement.hpp"
#include "charconv"
#include "Krystal.Booey/CSS/Values/ValueList.hpp"
#include "Krystal.Booey/HTML/Attributes/ReflectAttribute.hpp"
#include "Krystal.Booey/HTML/Internals/PresentationalHints.hpp"
#include "Krystal.Core/Text/ASCII.hpp"

namespace krys::boo::html
{
  HTMLFontElement::HTMLFontElement(dom::Document &document) noexcept
      : HTMLElement(document, HTMLElementInterface::Font)
  {
  }

  dom::DOMString HTMLFontElement::Color() const noexcept
  {
    return Reflect<dom::DOMString>(*this, u8"color").value_or(u8"");
  }

  void HTMLFontElement::Color(dom::DOMString value) noexcept
  {
    Reflect<dom::DOMString>(*this, u8"color", krys::move(value));
  }

  dom::DOMString HTMLFontElement::Face() const noexcept
  {
    return Reflect<dom::DOMString>(*this, u8"face").value_or(u8"");
  }

  void HTMLFontElement::Face(dom::DOMString value) noexcept
  {
    Reflect<dom::DOMString>(*this, u8"face", krys::move(value));
  }

  dom::DOMString HTMLFontElement::Size() const noexcept
  {
    return Reflect<dom::DOMString>(*this, u8"size").value_or(u8"");
  }

  void HTMLFontElement::Size(dom::DOMString value) noexcept
  {
    Reflect<dom::DOMString>(*this, u8"size", krys::move(value));
  }

  namespace
  {
    // https://html.spec.whatwg.org/#rules-for-parsing-a-legacy-font-size
    KRYS_NODISCARD Maybe<int32> ParseFontSize(dom::DOMStringView input) noexcept
    {
      // 1. Let input be the attribute's value.
      // 2. Let position be a pointer into input, initially pointing at the start of the string.
      size_t position = 0;

      // 3. Skip ASCII whitespace within input given position.
      while (position < input.size() && text::IsASCIIWhitespace(input[position]))
      {
        ++position;
      }

      // 4. If position is past the end of input, there is no presentational hint. Return.
      if (position == input.size())
      {
        return {};
      }

      // 5. If the character at position is a U+002B PLUS SIGN character (+), then let mode be relative-plus,
      //    and advance position to the next character. Otherwise, if the character at position is a U+002D
      //    HYPHEN-MINUS character (-), then let mode be relative-minus, and advance position to the next
      //    character. Otherwise, let mode be absolute.
      enum
      {
        RelativePlus,
        RelativeMinus,
        Absolute
      } mode;

      switch (input[position])
      {
        case '+':
        {
          mode = RelativePlus;
          ++position;
          break;
        }
        case '-':
        {
          mode = RelativeMinus;
          ++position;
          break;
        }
        default:
        {
          mode = Absolute;
          break;
        }
      }

      // 6. Collect a sequence of code points that are ASCII digits from input given position, and let digits
      // be
      //    the resulting sequence.
      auto digitsStart = position;
      while (position < input.size() && text::IsASCIIDigit(input[position]))
      {
        ++position;
      }

      // 7. If digits is the empty string, there is no presentational hint. Return.
      if (digitsStart == position)
      {
        return {};
      }

      // 8. Interpret digits as a base-ten integer. Let value be the resulting number.
      // (Every number past MaxValue clamps the same way, so larger ones are kept at it rather than
      // overflowing.)
      constexpr static int32 MaxValue = 100;
      int32 value = 0;
      const auto *first = reinterpret_cast<const char *>(input.data() + digitsStart);
      const auto *last = reinterpret_cast<const char *>(input.data() + position);
      if (std::from_chars(first, last, value).ec != std::errc {} || value > MaxValue)
      {
        value = MaxValue;
      }

      // 9. If mode is relative-plus, then increment value by 3. If mode is relative-minus, then let value be
      // the
      //    result of subtracting value from 3.
      if (mode == RelativePlus)
      {
        value += 3;
      }
      else if (mode == RelativeMinus)
      {
        value = 3 - value;
      }

      // 10. If value is greater than 7, let it be 7.
      if (value > 7)
      {
        value = 7;
      }

      // 11. If value is less than 1, let it be 1.
      if (value < 1)
      {
        value = 1;
      }

      return value;
    }

  }

  Maybe<css::ValueId> HTMLFontElement::CSSValueFromFontSizeNumber(dom::DOMStringView string) noexcept
  {
    auto number = ParseFontSize(string);
    if (!number)
    {
      return {};
    }

    // 12. Set the 'font-size' property to the keyword corresponding to the value of value according to the
    //     following table:
    switch (*number)
    {
      case 1:
      {
        return css::ValueId::XSmall;
      }
      case 2:
      {
        return css::ValueId::Small;
      }
      case 3:
      {
        return css::ValueId::Medium;
      }
      case 4:
      {
        return css::ValueId::Large;
      }
      case 5:
      {
        return css::ValueId::XLarge;
      }
      case 6:
      {
        return css::ValueId::XxLarge;
      }
      default:
      {
        return css::ValueId::XxxLarge;
      }
    }
  }

  bool HTMLFontElement::HasPresentationalHintsForAttribute(dom::DOMStringAtom localName,
                                                           dom::DOMStringAtom namespaceURI) const noexcept
  {
    if (namespaceURI == dom::DOMStringAtom::Null()
        && (localName == u8"size" || localName == u8"color" || localName == u8"face"))
    {
      return true;
    }
    return HTMLElement::HasPresentationalHintsForAttribute(localName, namespaceURI);
  }

  void HTMLFontElement::CollectPresentationalHintsForAttribute(dom::DOMStringAtom localName,
                                                               dom::DOMStringAtom namespaceURI,
                                                               dom::DOMStringView value,
                                                               css::MutableStyleProperties &style) noexcept
  {
    using namespace presentational_hints;
    if (namespaceURI != dom::DOMStringAtom::Null())
    {
      HTMLElement::CollectPresentationalHintsForAttribute(localName, namespaceURI, value, style);
      return;
    }
    if (localName == u8"size")
    {
      if (auto size = CSSValueFromFontSizeNumber(value))
      {
        AddPropertyToPresentationalHintStyle(style, css::PropertyId::FontSize, *size);
      }
      return;
    }
    if (localName == u8"color")
    {
      AddHTMLColorToStyle(style, css::PropertyId::Color, value);
      return;
    }
    if (localName == u8"face")
    {
      if (!value.empty())
      {
        if (auto fontFaceValue =
              css::ValuePool::MainThreadPool().CreateFontFaceValue(css::CSSOMStringAtom(value)))
        {
          style.SetProperty(css::PropertyId::FontFamily, Ref<css::Value> {krys::move(fontFaceValue)});
        }
      }
      return;
    }
    HTMLElement::CollectPresentationalHintsForAttribute(localName, namespaceURI, value, style);
  }
}
