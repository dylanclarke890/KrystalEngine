#include "Krystal.Booey/HTML/HTMLMarqueeElement.hpp"
#include "Krystal.Booey/HTML/Attributes/Reflection.hpp"
#include "Krystal.Booey/HTML/Internals/PresentationalHints.hpp"

namespace krys::boo::html
{
  HTMLMarqueeElement::HTMLMarqueeElement(dom::Document &document) noexcept
      : HTMLElement(document, HTMLElementInterface::Marquee)
  {
  }

  dom::DOMString HTMLMarqueeElement::Behavior() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"behavior").value_or(u8"");
  }

  void HTMLMarqueeElement::Behavior(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"behavior", krys::move(value));
  }

  dom::DOMString HTMLMarqueeElement::BgColor() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"bgcolor").value_or(u8"");
  }

  void HTMLMarqueeElement::BgColor(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"bgcolor", krys::move(value));
  }

  dom::DOMString HTMLMarqueeElement::Direction() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"direction").value_or(u8"");
  }

  void HTMLMarqueeElement::Direction(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"direction", krys::move(value));
  }

  dom::DOMString HTMLMarqueeElement::Height() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"height").value_or(u8"");
  }

  void HTMLMarqueeElement::Height(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"height", krys::move(value));
  }

  dom::DOMString HTMLMarqueeElement::Width() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"width").value_or(u8"");
  }

  void HTMLMarqueeElement::Width(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"width", krys::move(value));
  }

  bool HTMLMarqueeElement::HasPresentationalHintsForAttribute(dom::DOMStringAtom localName,
                                                              dom::DOMStringAtom namespaceURI) const noexcept
  {
    if (namespaceURI == dom::DOMStringAtom::Null()
        && (localName == u8"width" || localName == u8"height" || localName == u8"bgcolor"
            || localName == u8"vspace" || localName == u8"hspace"))
    {
      return true;
    }
    return HTMLElement::HasPresentationalHintsForAttribute(localName, namespaceURI);
  }

  void HTMLMarqueeElement::CollectPresentationalHintsForAttribute(dom::DOMStringAtom localName,
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
    if (localName == u8"width")
    {
      if (!value.empty())
      {
        AddHTMLLengthToStyle(style, css::PropertyId::Width, value);
      }
      return;
    }
    if (localName == u8"height")
    {
      if (!value.empty())
      {
        AddHTMLLengthToStyle(style, css::PropertyId::Height, value);
      }
      return;
    }
    if (localName == u8"bgcolor")
    {
      if (!value.empty())
      {
        AddHTMLColorToStyle(style, css::PropertyId::BackgroundColor, value);
      }
      return;
    }
    if (localName == u8"vspace")
    {
      if (!value.empty())
      {
        AddHTMLLengthToStyle(style, css::PropertyId::MarginTop, value);
        AddHTMLLengthToStyle(style, css::PropertyId::MarginBottom, value);
      }
      return;
    }
    if (localName == u8"hspace")
    {
      if (!value.empty())
      {
        AddHTMLLengthToStyle(style, css::PropertyId::MarginLeft, value);
        AddHTMLLengthToStyle(style, css::PropertyId::MarginRight, value);
      }
      return;
    }
    HTMLElement::CollectPresentationalHintsForAttribute(localName, namespaceURI, value, style);
  }
}
