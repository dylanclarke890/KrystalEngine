#include "Krystal.Booey/HTML/Elements/HTMLBodyElement.hpp"
#include "Krystal.Booey/CSS/Properties/MutableStyleProperties.hpp"
#include "Krystal.Booey/CSS/Values/ImageValue.hpp"
#include "Krystal.Booey/DOM/Nodes/Document.hpp"
#include "Krystal.Booey/HTML/Attributes/ReflectAttribute.hpp"
#include "Krystal.Booey/HTML/Internals/PresentationalHints.hpp"

namespace krys::boo
{
  HTMLBodyElement::HTMLBodyElement(Document &document) noexcept
      : HTMLElement(document, html::HTMLElementInterface::Body)
  {
  }

#pragma region HTMLBodyElement Obsolete members

  DOMString HTMLBodyElement::Text() const noexcept
  {
    return html::Reflect<DOMString>(*this, u8"text").value_or(u8"");
  }

  void HTMLBodyElement::Text(DOMString value) noexcept
  {
    html::Reflect<DOMString>(*this, u8"text", krys::move(value));
  }

  DOMString HTMLBodyElement::Link() const noexcept
  {
    return html::Reflect<DOMString>(*this, u8"link").value_or(u8"");
  }

  void HTMLBodyElement::Link(DOMString value) noexcept
  {
    html::Reflect<DOMString>(*this, u8"link", krys::move(value));
  }

  DOMString HTMLBodyElement::VLink() const noexcept
  {
    return html::Reflect<DOMString>(*this, u8"vlink").value_or(u8"");
  }

  void HTMLBodyElement::VLink(DOMString value) noexcept
  {
    html::Reflect<DOMString>(*this, u8"vlink", krys::move(value));
  }

  DOMString HTMLBodyElement::ALink() const noexcept
  {
    return html::Reflect<DOMString>(*this, u8"alink").value_or(u8"");
  }

  void HTMLBodyElement::ALink(DOMString value) noexcept
  {
    html::Reflect<DOMString>(*this, u8"alink", krys::move(value));
  }

  DOMString HTMLBodyElement::BgColor() const noexcept
  {
    return html::Reflect<DOMString>(*this, u8"bgcolor").value_or(u8"");
  }

  void HTMLBodyElement::BgColor(DOMString value) noexcept
  {
    html::Reflect<DOMString>(*this, u8"bgcolor", krys::move(value));
  }

  DOMString HTMLBodyElement::Background() const noexcept
  {
    return html::Reflect<DOMString>(*this, u8"background").value_or(u8"");
  }

  void HTMLBodyElement::Background(DOMString value) noexcept
  {
    html::Reflect<DOMString>(*this, u8"background", krys::move(value));
  }

#pragma endregion

#pragma region Presentational hints

  bool HTMLBodyElement::HasPresentationalHintsForAttribute(const DOMStringAtom &localName,
                                                           const DOMStringAtom &namespaceURI) const noexcept
  {
    if (namespaceURI == null
        && (localName == u8"background" || localName == u8"marginwidth" || localName == u8"leftmargin"
            || localName == u8"marginheight" || localName == u8"topmargin" || localName == u8"bgcolor"
            || localName == u8"text"))
    {
      return true;
    }
    return HTMLElement::HasPresentationalHintsForAttribute(localName, namespaceURI);
  }

  void HTMLBodyElement::CollectPresentationalHintsForAttribute(const DOMStringAtom &localName,
                                                               const DOMStringAtom &namespaceURI,
                                                               DOMStringView value,
                                                               css::MutableStyleProperties &style) noexcept
  {
    using namespace html::presentational_hints;
    if (namespaceURI != null)
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
          css::ImageValue::Create(NodeDocument().CompleteUrl(url), CSSOMStringAtom(LocalName().View())));
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

  void HTMLBodyElement::OnAttributeChanged(const DOMStringAtom &localName, Maybe<DOMStringView> oldValue,
                                           Maybe<DOMStringView> value,
                                           const DOMStringAtom &namespaceURI) noexcept
  {
    if (namespaceURI == null)
    {
      auto &document = NodeDocument();
      auto parsedColor = value ? ParseLegacyColorValue(*value) : Maybe<SRGBA<uint8>> {};
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
