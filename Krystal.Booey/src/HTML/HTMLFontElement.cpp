#include "Krystal.Booey/HTML/HTMLFontElement.hpp"
#include "charconv"
#include "Krystal.Booey/CSS/Values/ValueList.hpp"
#include "Krystal.Booey/HTML/Attributes/Reflection.hpp"
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
    return Reflection::Reflect<dom::DOMString>(*this, u8"color").value_or(u8"");
  }

  void HTMLFontElement::Color(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"color", krys::move(value));
  }

  dom::DOMString HTMLFontElement::Face() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"face").value_or(u8"");
  }

  void HTMLFontElement::Face(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"face", krys::move(value));
  }

  dom::DOMString HTMLFontElement::Size() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"size").value_or(u8"");
  }

  void HTMLFontElement::Size(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"size", krys::move(value));
  }

  namespace
  {
    // https://html.spec.whatwg.org/#rules-for-parsing-a-legacy-font-size
    KRYS_NODISCARD Maybe<int32> ParseFontSize(dom::DOMStringView input) noexcept
    {
      // Step 1
      // Step 2
      // Step 3
      size_t position = 0;
      while (position < input.size() && text::IsASCIIWhitespace(input[position]))
      {
        ++position;
      }

      // Step 4
      if (position == input.size())
      {
        return {};
      }

      // Step 5
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

      // Step 6
      auto digitsStart = position;
      while (position < input.size() && text::IsASCIIDigit(input[position]))
      {
        ++position;
      }

      // Step 7
      if (digitsStart == position)
      {
        return {};
      }

      // Step 8
      int32 value = 0;
      const auto *first = reinterpret_cast<const char *>(input.data() + digitsStart);
      const auto *last = reinterpret_cast<const char *>(input.data() + position);
      if (std::from_chars(first, last, value).ec != std::errc {})
      {
        value = 0;
      }

      // Step 9
      if (mode == RelativePlus)
      {
        value += 3;
      }
      else if (mode == RelativeMinus)
      {
        value = 3 - value;
      }

      // Step 10
      if (value > 7)
      {
        value = 7;
      }

      // Step 11
      if (value < 1)
      {
        value = 1;
      }

      return value;
    }

    KRYS_NODISCARD Maybe<css::ValueId> CSSValueFromFontSizeNumber(dom::DOMStringView string) noexcept
    {
      auto number = ParseFontSize(string);
      if (!number)
      {
        return {};
      }

      switch (*number)
      {
        case 1:
        {
          // FIXME: The spec says that we're supposed to use CSSValueXxSmall here.
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
