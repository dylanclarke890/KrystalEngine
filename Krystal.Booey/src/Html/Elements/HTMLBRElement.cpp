#include "Krystal.Booey/Html/Elements/HTMLBRElement.hpp"
#include "Krystal.Booey/Html/Attributes/ReflectAttribute.hpp"
#include "Krystal.Booey/Html/Internals/PresentationalHints.hpp"
#include "Krystal.Booey/Infra/StringAlgorithms.hpp"
#include "Krystal.Booey/Rendering/RenderLineBreak.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"

namespace krys::boo
{
  HTMLBRElement::HTMLBRElement(Document &document) noexcept
      : HTMLElement(document, html::HTMLElementInterface::BR)
  {
  }

  RenderPtr<RenderElement> HTMLBRElement::CreateElementRenderer(RenderStyle &&style) noexcept
  {
    if (RenderElement::IsContentDataSupported(style.Content()))
    {
      return RenderElement::CreateFor(*this, krys::move(style));
    }

    return CreateRenderer<RenderLineBreak>(*this, krys::move(style));
  }

#pragma region HTMLBRElement Obsolete members

  DOMString HTMLBRElement::Clear() const noexcept
  {
    return html::Reflect<DOMString>(*this, u8"clear").value_or(u8"");
  }

  void HTMLBRElement::Clear(DOMString value) noexcept
  {
    html::Reflect<DOMString>(*this, u8"clear", krys::move(value));
  }

#pragma endregion

#pragma region Presentational hints

  bool HTMLBRElement::HasPresentationalHintsForAttribute(const DOMStringAtom &localName,
                                                         const DOMStringAtom &namespaceURI) const noexcept
  {
    if (namespaceURI == null && (localName == u8"clear"))
    {
      return true;
    }
    return HTMLElement::HasPresentationalHintsForAttribute(localName, namespaceURI);
  }

  void HTMLBRElement::CollectPresentationalHintsForAttribute(const DOMStringAtom &localName,
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
    if (localName == u8"clear")
    {
      // An empty clear adds nothing; all means both, and anything else is tried as a clear value.
      if (!value.empty())
      {
        if (infra::StringAlgorithms::ASCIICaseInsensitiveMatch(value, u8"all"))
        {
          AddPropertyToPresentationalHintStyle(style, css::PropertyId::Clear, css::ValueId::Both);
        }
        else
        {
          AddPropertyToPresentationalHintStyle(style, css::PropertyId::Clear, ToCSSOMString(value));
        }
      }
      return;
    }
    HTMLElement::CollectPresentationalHintsForAttribute(localName, namespaceURI, value, style);
  }

#pragma endregion
}
