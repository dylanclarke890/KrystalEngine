#include "Krystal.Booey/Html/Elements/HTMLDivElement.hpp"
#include "Krystal.Booey/Html/Attributes/ReflectAttribute.hpp"
#include "Krystal.Booey/Html/Internals/PresentationalHints.hpp"
#include "Krystal.Booey/Infra/StringAlgorithms.hpp"

namespace krys::boo
{
  HTMLDivElement::HTMLDivElement(Document &document) noexcept
      : HTMLElement(document, html::HTMLElementInterface::Div, u8"div", {}, dom::NodeFlags {})
  {
  }

  HTMLDivElement::HTMLDivElement(Document &document, dom::NodeFlags nodeFlags) noexcept
      : HTMLElement(document, html::HTMLElementInterface::Div, u8"div", {}, nodeFlags)
  {
  }

#pragma region HTMLDivElement Obsolete members

  DOMString HTMLDivElement::Align() const noexcept
  {
    return html::Reflect<DOMString>(*this, u8"align").value_or(u8"");
  }

  void HTMLDivElement::Align(DOMString value) noexcept
  {
    html::Reflect<DOMString>(*this, u8"align", krys::move(value));
  }

#pragma endregion

#pragma region Presentational hints

  void HTMLDivElement::CollectPresentationalHintsForAttribute(const DOMStringAtom &localName,
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
    if (localName == u8"align")
    {
      if (infra::StringAlgorithms::ASCIICaseInsensitiveMatch(value, u8"middle")
          || infra::StringAlgorithms::ASCIICaseInsensitiveMatch(value, u8"center"))
      {
        AddPropertyToPresentationalHintStyle(style, css::PropertyId::TextAlign, css::ValueId::InternalCenter);
      }
      else if (infra::StringAlgorithms::ASCIICaseInsensitiveMatch(value, u8"left"))
      {
        AddPropertyToPresentationalHintStyle(style, css::PropertyId::TextAlign, css::ValueId::InternalLeft);
      }
      else if (infra::StringAlgorithms::ASCIICaseInsensitiveMatch(value, u8"right"))
      {
        AddPropertyToPresentationalHintStyle(style, css::PropertyId::TextAlign, css::ValueId::InternalRight);
      }
      else
      {
        AddPropertyToPresentationalHintStyle(style, css::PropertyId::TextAlign, ToCSSOMString(value));
      }
      return;
    }
    HTMLElement::CollectPresentationalHintsForAttribute(localName, namespaceURI, value, style);
  }

#pragma endregion
}
