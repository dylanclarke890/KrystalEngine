#include "Krystal.Web/Html/Elements/HTMLIFrameElement.hpp"
#include "Krystal.Web/Html/Attributes/ReflectAttribute.hpp"
#include "Krystal.Web/Html/Internals/PresentationalHints.hpp"
#include "Krystal.Web/Html/MicroParsers/Numbers.hpp"

namespace krys::web
{
  HTMLIFrameElement::HTMLIFrameElement(Document &document) noexcept
      : HTMLElement(document, html::HTMLElementInterface::IFrame)
  {
  }

#pragma region HTMLIFrameElement

  dom::USVString HTMLIFrameElement::Src() const noexcept
  {
    return html::ReflectUrl<dom::USVString>(*this, u8"src");
  }

  void HTMLIFrameElement::Src(dom::USVString value) noexcept
  {
    return html::ReflectUrl<dom::USVString>(*this, u8"src", krys::move(value));
  }

  DOMString HTMLIFrameElement::SrcDoc() const noexcept
  {
    return html::Reflect<DOMString>(*this, u8"srcdoc").value_or(u8"");
  }

  void HTMLIFrameElement::SrcDoc(DOMString value) noexcept
  {
    html::Reflect<DOMString>(*this, u8"srcdoc", krys::move(value));
  }

  DOMString HTMLIFrameElement::Sizes() const noexcept
  {
    return html::Reflect<DOMString>(*this, u8"sizes").value_or(u8"");
  }

  void HTMLIFrameElement::Sizes(DOMString value) noexcept
  {
    html::Reflect<DOMString>(*this, u8"sizes", krys::move(value));
  }

  DOMString HTMLIFrameElement::Name() const noexcept
  {
    return html::Reflect<DOMString>(*this, u8"name").value_or(u8"");
  }

  void HTMLIFrameElement::Name(DOMString value) noexcept
  {
    html::Reflect<DOMString>(*this, u8"name", krys::move(value));
  }

  DOMString HTMLIFrameElement::Allow() const noexcept
  {
    return html::Reflect<DOMString>(*this, u8"allow").value_or(u8"");
  }

  void HTMLIFrameElement::Allow(DOMString value) noexcept
  {
    html::Reflect<DOMString>(*this, u8"allow", krys::move(value));
  }

  bool HTMLIFrameElement::AllowFullscreen() const noexcept
  {
    return html::Reflect<bool>(*this, u8"allowfullscreen");
  }

  void HTMLIFrameElement::AllowFullscreen(bool value) noexcept
  {
    html::Reflect<bool>(*this, u8"allowfullscreen", krys::move(value));
  }

  DOMString HTMLIFrameElement::Width() const noexcept
  {
    return html::Reflect<DOMString>(*this, u8"width").value_or(u8"");
  }

  void HTMLIFrameElement::Width(DOMString value) noexcept
  {
    html::Reflect<DOMString>(*this, u8"width", krys::move(value));
  }

  DOMString HTMLIFrameElement::Height() const noexcept
  {
    return html::Reflect<DOMString>(*this, u8"height").value_or(u8"");
  }

  void HTMLIFrameElement::Height(DOMString value) noexcept
  {
    html::Reflect<DOMString>(*this, u8"height", krys::move(value));
  }

  DOMString HTMLIFrameElement::ReferrerPolicy() const noexcept
  {
    return html::ReflectEnumerated<"referrerpolicy">(*this);
  }

  void HTMLIFrameElement::ReferrerPolicy(DOMString value) noexcept
  {
    html::Reflect<DOMString>(*this, u8"referrerpolicy", krys::move(value));
  }

  DOMString HTMLIFrameElement::Loading() const noexcept
  {
    return html::ReflectEnumerated<"loading">(*this);
  }

  void HTMLIFrameElement::Loading(DOMString value) noexcept
  {
    html::Reflect<DOMString>(*this, u8"loading", krys::move(value));
  }

#pragma endregion

#pragma region HTMLIFrameElement Obsolete members

  DOMString HTMLIFrameElement::Align() const noexcept
  {
    return html::Reflect<DOMString>(*this, u8"align").value_or(u8"");
  }

  void HTMLIFrameElement::Align(DOMString value) noexcept
  {
    html::Reflect<DOMString>(*this, u8"align", krys::move(value));
  }

  DOMString HTMLIFrameElement::Scrolling() const noexcept
  {
    return html::Reflect<DOMString>(*this, u8"scrolling").value_or(u8"");
  }

  void HTMLIFrameElement::Scrolling(DOMString value) noexcept
  {
    html::Reflect<DOMString>(*this, u8"scrolling", krys::move(value));
  }

  DOMString HTMLIFrameElement::FrameBorder() const noexcept
  {
    return html::Reflect<DOMString>(*this, u8"frameborder").value_or(u8"");
  }

  void HTMLIFrameElement::FrameBorder(DOMString value) noexcept
  {
    html::Reflect<DOMString>(*this, u8"frameborder", krys::move(value));
  }

  dom::USVString HTMLIFrameElement::LongDesc() const noexcept
  {
    return html::ReflectUrl<dom::USVString>(*this, u8"longdesc");
  }

  void HTMLIFrameElement::LongDesc(dom::USVString value) noexcept
  {
    return html::ReflectUrl<dom::USVString>(*this, u8"longdesc", krys::move(value));
  }

  DOMString HTMLIFrameElement::MarginWidth() const noexcept
  {
    return html::Reflect<DOMString>(*this, u8"marginwidth").value_or(u8"");
  }

  void HTMLIFrameElement::MarginWidth(DOMString value) noexcept
  {
    html::Reflect<DOMString>(*this, u8"marginwidth", krys::move(value));
  }

  DOMString HTMLIFrameElement::MarginHeight() const noexcept
  {
    return html::Reflect<DOMString>(*this, u8"marginheight").value_or(u8"");
  }

  void HTMLIFrameElement::MarginHeight(DOMString value) noexcept
  {
    html::Reflect<DOMString>(*this, u8"marginheight", krys::move(value));
  }

#pragma endregion

#pragma region Presentational hints

  bool HTMLIFrameElement::HasPresentationalHintsForAttribute(const DOMStringAtom &localName,
                                                             const DOMStringAtom &namespaceURI) const noexcept
  {
    if (namespaceURI == null
        && (localName == u8"width" || localName == u8"height" || localName == u8"frameborder"))
    {
      return true;
    }
    return HTMLElement::HasPresentationalHintsForAttribute(localName, namespaceURI);
  }

  void HTMLIFrameElement::CollectPresentationalHintsForAttribute(const DOMStringAtom &localName,
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
    if (localName == u8"width")
    {
      AddHTMLLengthToStyle(style, css::PropertyId::Width, value);
      return;
    }
    if (localName == u8"height")
    {
      AddHTMLLengthToStyle(style, css::PropertyId::Height, value);
      return;
    }
    if (localName == u8"align")
    {
      ApplyAlignmentAttributeToStyle(value, style);
      return;
    }
    if (localName == u8"frameborder")
    {
      // A frameborder that's zero, or not an integer, removes the border.
      auto frameBorder = html::Numbers::ParseInteger(value);
      if (!frameBorder.Success() || !frameBorder.Value)
      {
        AddPropertyToPresentationalHintStyle(style, css::PropertyId::BorderWidth, 0, css::UnitType::px);
      }
      return;
    }
    HTMLElement::CollectPresentationalHintsForAttribute(localName, namespaceURI, value, style);
  }

#pragma endregion
}
