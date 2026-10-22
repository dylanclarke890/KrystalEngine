#include "Krystal.Booey/HTML/HTMLTablePartElement.hpp"
#include "Krystal.Booey/CSS/Values/ImageValue.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/HTML/HTMLTableElement.hpp"
#include "Krystal.Booey/HTML/Internals/PresentationalHints.hpp"
#include "Krystal.Booey/Infra/StringAlgorithms.hpp"
#include "Krystal.Core/TypeCast.hpp"

namespace krys::boo::html
{
  HTMLTablePartElement::HTMLTablePartElement(dom::Document &document, HTMLElementInterface interface) noexcept
      : HTMLElement(document, interface)
  {
  }

  const HTMLTableElement *HTMLTablePartElement::FindParentTable() const noexcept
  {
    for (auto *ancestor = ParentElement(); ancestor; ancestor = ancestor->ParentElement())
    {
      auto *htmlElement = DynamicDowncast<HTMLElement>(*ancestor);
      if (auto *table = htmlElement ? DynamicDowncast<HTMLTableElement>(*htmlElement) : nullptr)
      {
        return table;
      }
    }
    return nullptr;
  }

  bool
    HTMLTablePartElement::HasPresentationalHintsForAttribute(dom::DOMStringAtom localName,
                                                             dom::DOMStringAtom namespaceURI) const noexcept
  {
    if (namespaceURI == dom::DOMStringAtom::Null()
        && (localName == u8"bgcolor" || localName == u8"background" || localName == u8"valign"
            || localName == u8"height"))
    {
      return true;
    }
    return HTMLElement::HasPresentationalHintsForAttribute(localName, namespaceURI);
  }

  void HTMLTablePartElement::CollectPresentationalHintsForAttribute(
    dom::DOMStringAtom localName, dom::DOMStringAtom namespaceURI, dom::DOMStringView value,
    css::MutableStyleProperties &style) noexcept
  {
    using namespace presentational_hints;
    if (namespaceURI != dom::DOMStringAtom::Null())
    {
      HTMLElement::CollectPresentationalHintsForAttribute(localName, namespaceURI, value, style);
      return;
    }
    if (localName == u8"bgcolor")
    {
      AddHTMLColorToStyle(style, css::PropertyId::BackgroundColor, value);
      return;
    }
    if (localName == u8"background")
    {
      if (!infra::StringAlgorithms::StripLeadingAndTrailingWhitespace(value).empty())
      {
        style.SetProperty(css::PropertyId::BackgroundImage,
                          css::ImageValue::Create(NodeDocument().CompleteURL(value)));
      }
      return;
    }
    if (localName == u8"valign")
    {
      if (infra::StringAlgorithms::ASCIICaseInsensitiveMatch(value, u8"top"))
      {
        AddPropertyToPresentationalHintStyle(style, css::PropertyId::VerticalAlign, css::ValueId::Top);
      }
      else if (infra::StringAlgorithms::ASCIICaseInsensitiveMatch(value, u8"middle"))
      {
        AddPropertyToPresentationalHintStyle(style, css::PropertyId::VerticalAlign, css::ValueId::Middle);
      }
      else if (infra::StringAlgorithms::ASCIICaseInsensitiveMatch(value, u8"bottom"))
      {
        AddPropertyToPresentationalHintStyle(style, css::PropertyId::VerticalAlign, css::ValueId::Bottom);
      }
      else if (infra::StringAlgorithms::ASCIICaseInsensitiveMatch(value, u8"baseline"))
      {
        AddPropertyToPresentationalHintStyle(style, css::PropertyId::VerticalAlign, css::ValueId::Baseline);
      }
      else
      {
        AddPropertyToPresentationalHintStyle(style, css::PropertyId::VerticalAlign, ToCSSOMString(value));
      }
      return;
    }
    if (localName == u8"align")
    {
      if (infra::StringAlgorithms::ASCIICaseInsensitiveMatch(value, u8"middle")
          || infra::StringAlgorithms::ASCIICaseInsensitiveMatch(value, u8"center"))
      {
        AddPropertyToPresentationalHintStyle(style, css::PropertyId::TextAlign, css::ValueId::WebkitCenter);
      }
      else if (infra::StringAlgorithms::ASCIICaseInsensitiveMatch(value, u8"absmiddle"))
      {
        AddPropertyToPresentationalHintStyle(style, css::PropertyId::TextAlign, css::ValueId::Center);
      }
      else if (infra::StringAlgorithms::ASCIICaseInsensitiveMatch(value, u8"left"))
      {
        AddPropertyToPresentationalHintStyle(style, css::PropertyId::TextAlign, css::ValueId::WebkitLeft);
      }
      else if (infra::StringAlgorithms::ASCIICaseInsensitiveMatch(value, u8"right"))
      {
        AddPropertyToPresentationalHintStyle(style, css::PropertyId::TextAlign, css::ValueId::WebkitRight);
      }
      else
      {
        AddPropertyToPresentationalHintStyle(style, css::PropertyId::TextAlign, ToCSSOMString(value));
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
    HTMLElement::CollectPresentationalHintsForAttribute(localName, namespaceURI, value, style);
  }
}
