#include "Krystal.Booey/HTML/HTMLBodyElement.hpp"
#include "Krystal.Booey/CSS/Values/ImageValue.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/HTML/Attributes/Reflection.hpp"
#include "Krystal.Booey/HTML/Internals/PresentationalHints.hpp"

namespace krys::boo::html
{
  HTMLBodyElement::HTMLBodyElement(dom::Document &document) noexcept
      : HTMLElement(document, HTMLElementInterface::Body)
  {
  }

#pragma region HTMLBodyElement Obsolete members

  dom::DOMString HTMLBodyElement::Text() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"text").value_or(u8"");
  }

  void HTMLBodyElement::Text(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"text", krys::move(value));
  }

  dom::DOMString HTMLBodyElement::Link() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"link").value_or(u8"");
  }

  void HTMLBodyElement::Link(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"link", krys::move(value));
  }

  dom::DOMString HTMLBodyElement::VLink() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"vlink").value_or(u8"");
  }

  void HTMLBodyElement::VLink(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"vlink", krys::move(value));
  }

  dom::DOMString HTMLBodyElement::ALink() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"alink").value_or(u8"");
  }

  void HTMLBodyElement::ALink(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"alink", krys::move(value));
  }

  dom::DOMString HTMLBodyElement::BgColor() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"bgcolor").value_or(u8"");
  }

  void HTMLBodyElement::BgColor(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"bgcolor", krys::move(value));
  }

  dom::DOMString HTMLBodyElement::Background() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"background").value_or(u8"");
  }

  void HTMLBodyElement::Background(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"background", krys::move(value));
  }

#pragma endregion

#pragma region Presentational hints

  bool HTMLBodyElement::HasPresentationalHintsForAttribute(dom::DOMStringAtom localName,
                                                           dom::DOMStringAtom namespaceURI) const noexcept
  {
    if (namespaceURI == dom::DOMStringAtom::Null()
        && (localName == u8"background" || localName == u8"marginwidth" || localName == u8"leftmargin"
            || localName == u8"marginheight" || localName == u8"topmargin" || localName == u8"bgcolor"
            || localName == u8"text"))
    {
      return true;
    }
    return HTMLElement::HasPresentationalHintsForAttribute(localName, namespaceURI);
  }

  void HTMLBodyElement::CollectPresentationalHintsForAttribute(dom::DOMStringAtom localName,
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
    if (localName == u8"background")
    {
      auto url = infra::StringAlgorithms::StripLeadingAndTrailingWhitespace(value);
      if (!url.empty())
      {
        style.SetProperty(
          css::PropertyId::BackgroundImage,
          css::ImageValue::Create(NodeDocument().CompleteURL(url), css::CSSOMStringAtom(LocalName().View())));
      }
      return;
    }
    if (localName == u8"marginwidth" || localName == u8"leftmargin")
    {
      AddHTMLLengthToStyle(style, css::PropertyId::MarginRight, value);
      AddHTMLLengthToStyle(style, css::PropertyId::MarginLeft, value);
      return;
    }
    if (localName == u8"marginheight" || localName == u8"topmargin")
    {
      AddHTMLLengthToStyle(style, css::PropertyId::MarginBottom, value);
      AddHTMLLengthToStyle(style, css::PropertyId::MarginTop, value);
      return;
    }
    if (localName == u8"bgcolor")
    {
      AddHTMLColorToStyle(style, css::PropertyId::BackgroundColor, value);
      return;
    }
    if (localName == u8"text")
    {
      AddHTMLColorToStyle(style, css::PropertyId::Color, value);
      return;
    }
    HTMLElement::CollectPresentationalHintsForAttribute(localName, namespaceURI, value, style);
  }

  void HTMLBodyElement::OnAttributeChanged(dom::DOMStringAtom localName, Maybe<dom::DOMStringView> oldValue,
                                           Maybe<dom::DOMStringView> value,
                                           dom::DOMStringAtom namespaceURI) noexcept
  {
    if (namespaceURI == dom::DOMStringAtom::Null())
    {
      auto &document = NodeDocument();
      auto parsedColor = value ? ParseLegacyColorValue(*value) : Maybe<SRGBA<uint8>> {};
      if (localName == u8"vlink")
      {
        if (parsedColor)
        {
          document.SetVisitedLinkColor(*parsedColor);
        }
        else
        {
          document.ResetVisitedLinkColor();
        }
        InvalidateStyleForSubtree();
        return;
      }
      if (localName == u8"alink")
      {
        if (parsedColor)
        {
          document.SetActiveLinkColor(*parsedColor);
        }
        else
        {
          document.ResetActiveLinkColor();
        }
        InvalidateStyleForSubtree();
        return;
      }
      if (localName == u8"link")
      {
        if (parsedColor)
        {
          document.SetLinkColor(*parsedColor);
        }
        else
        {
          document.ResetLinkColor();
        }
        InvalidateStyleForSubtree();
        return;
      }
    }

    HTMLElement::OnAttributeChanged(localName, oldValue, value, namespaceURI);
  }

#pragma endregion
}
