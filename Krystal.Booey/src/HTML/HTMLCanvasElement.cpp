#include "Krystal.Booey/HTML/HTMLCanvasElement.hpp"
#include "Krystal.Booey/HTML/Attributes/Reflection.hpp"
#include "Krystal.Booey/HTML/Internals/PresentationalHints.hpp"

namespace krys::boo::html
{
  HTMLCanvasElement::HTMLCanvasElement(dom::Document &document) noexcept
      : HTMLElement(document, HTMLElementInterface::Canvas)
  {
  }

#pragma region HTMLCanvasElement

  uint32 HTMLCanvasElement::Width() const noexcept
  {
    return Reflection::Reflect<uint32>(*this, u8"width");
  }

  void HTMLCanvasElement::Width(uint32 value) noexcept
  {
    Reflection::Reflect<uint32>(*this, u8"width", krys::move(value));
  }

  uint32 HTMLCanvasElement::Height() const noexcept
  {
    return Reflection::Reflect<uint32>(*this, u8"height");
  }

  void HTMLCanvasElement::Height(uint32 value) noexcept
  {
    Reflection::Reflect<uint32>(*this, u8"height", krys::move(value));
  }

#pragma endregion

#pragma region Presentational hints

  bool HTMLCanvasElement::HasPresentationalHintsForAttribute(dom::DOMStringAtom localName,
                                                             dom::DOMStringAtom namespaceURI) const noexcept
  {
    if (namespaceURI == dom::DOMStringAtom::Null() && (localName == u8"width" || localName == u8"height"))
    {
      return true;
    }
    return HTMLElement::HasPresentationalHintsForAttribute(localName, namespaceURI);
  }

  void HTMLCanvasElement::CollectPresentationalHintsForAttribute(dom::DOMStringAtom localName,
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
      ApplyAspectRatioWithoutDimensionalRulesFromWidthAndHeightAttributesToStyle(
        value, AttributeOrEmpty(u8"height"), style);
      return;
    }
    if (localName == u8"height")
    {
      ApplyAspectRatioWithoutDimensionalRulesFromWidthAndHeightAttributesToStyle(AttributeOrEmpty(u8"width"),
                                                                                 value, style);
      return;
    }
    HTMLElement::CollectPresentationalHintsForAttribute(localName, namespaceURI, value, style);
  }

#pragma endregion
}
