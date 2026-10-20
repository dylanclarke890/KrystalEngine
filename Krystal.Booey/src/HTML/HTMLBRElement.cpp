#include "Krystal.Booey/HTML/HTMLBRElement.hpp"
#include "Krystal.Booey/HTML/Attributes/Reflection.hpp"
#include "Krystal.Booey/HTML/Internals/PresentationalHints.hpp"
#include "Krystal.Booey/Rendering/RenderLineBreak.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"

namespace krys::boo::html
{
  HTMLBRElement::HTMLBRElement(dom::Document &document) noexcept
      : HTMLElement(document, HTMLElementInterface::BR)
  {
  }

  RenderPtr<RenderElement> HTMLBRElement::CreateElementRenderer(RenderStyle &&style) noexcept
  {
    if (RenderElement::IsContentDataSupported(style.GetContent()))
    {
      return RenderElement::CreateFor(*this, krys::move(style));
    }

    return CreateRenderer<RenderLineBreak>(*this, krys::move(style));
  }

#pragma region HTMLBRElement Obsolete members

  dom::DOMString HTMLBRElement::Clear() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"clear").value_or(u8"");
  }

  void HTMLBRElement::Clear(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"clear", krys::move(value));
  }

#pragma endregion

#pragma region Presentational hints

  bool HTMLBRElement::HasPresentationalHintsForAttribute(dom::DOMStringAtom localName,
                                                         dom::DOMStringAtom namespaceURI) const noexcept
  {
    if (namespaceURI == dom::DOMStringAtom::Null() && (localName == u8"clear"))
    {
      return true;
    }
    return HTMLElement::HasPresentationalHintsForAttribute(localName, namespaceURI);
  }

  void HTMLBRElement::CollectPresentationalHintsForAttribute(dom::DOMStringAtom localName,
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
    if (localName == u8"clear")
    {
      // If the string is empty, then don't add the clear property.
      // <br clear> and <br clear=""> are just treated like <br> by Gecko, Mac IE, etc. -dwh
      if (!value.empty())
      {
        if (EqualLettersIgnoringASCIICase(value, u8"all"))
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
