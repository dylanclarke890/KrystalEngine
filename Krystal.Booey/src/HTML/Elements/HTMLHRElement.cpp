#include "Krystal.Booey/HTML/Elements/HTMLHRElement.hpp"
#include "Krystal.Booey/CSS/Properties/MutableStyleProperties.hpp"
#include "Krystal.Booey/CSS/Values/ColorValue.hpp"
#include "Krystal.Booey/CSS/Values/ValuePool.hpp"
#include "Krystal.Booey/HTML/Attributes/ReflectAttribute.hpp"
#include "Krystal.Booey/HTML/Internals/PresentationalHints.hpp"
#include "Krystal.Booey/HTML/MicroParsers/Numbers.hpp"
#include "Krystal.Booey/Infra/StringAlgorithms.hpp"

namespace krys::boo
{
  HTMLHRElement::HTMLHRElement(Document &document) noexcept
      : HTMLElement(document, html::HTMLElementInterface::HR)
  {
  }

#pragma region HTMLHRElement Obsolete members

  DOMString HTMLHRElement::Align() const noexcept
  {
    return html::Reflect<DOMString>(*this, u8"align").value_or(u8"");
  }

  void HTMLHRElement::Align(DOMString value) noexcept
  {
    html::Reflect<DOMString>(*this, u8"align", krys::move(value));
  }

  DOMString HTMLHRElement::Color() const noexcept
  {
    return html::Reflect<DOMString>(*this, u8"color").value_or(u8"");
  }

  void HTMLHRElement::Color(DOMString value) noexcept
  {
    html::Reflect<DOMString>(*this, u8"color", krys::move(value));
  }

  bool HTMLHRElement::NoShade() const noexcept
  {
    return html::Reflect<bool>(*this, u8"noshade");
  }

  void HTMLHRElement::NoShade(bool value) noexcept
  {
    html::Reflect<bool>(*this, u8"noshade", krys::move(value));
  }

  DOMString HTMLHRElement::Size() const noexcept
  {
    return html::Reflect<DOMString>(*this, u8"size").value_or(u8"");
  }

  void HTMLHRElement::Size(DOMString value) noexcept
  {
    html::Reflect<DOMString>(*this, u8"size", krys::move(value));
  }

  DOMString HTMLHRElement::Width() const noexcept
  {
    return html::Reflect<DOMString>(*this, u8"width").value_or(u8"");
  }

  void HTMLHRElement::Width(DOMString value) noexcept
  {
    html::Reflect<DOMString>(*this, u8"width", krys::move(value));
  }

#pragma endregion

#pragma region Presentational hints

  bool HTMLHRElement::HasPresentationalHintsForAttribute(const DOMStringAtom &localName,
                                                         const DOMStringAtom &namespaceURI) const noexcept
  {
    if (namespaceURI == null
        && (localName == u8"width" || localName == u8"color" || localName == u8"noshade"
            || localName == u8"size"))
    {
      return true;
    }
    return HTMLElement::HasPresentationalHintsForAttribute(localName, namespaceURI);
  }

  void HTMLHRElement::CollectPresentationalHintsForAttribute(const DOMStringAtom &localName,
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
      if (infra::StringAlgorithms::ASCIICaseInsensitiveMatch(value, u8"left"))
      {
        AddPropertyToPresentationalHintStyle(style, css::PropertyId::MarginLeft, 0, css::UnitType::px);
        AddPropertyToPresentationalHintStyle(style, css::PropertyId::MarginRight, css::ValueId::Auto);
      }
      else if (infra::StringAlgorithms::ASCIICaseInsensitiveMatch(value, u8"right"))
      {
        AddPropertyToPresentationalHintStyle(style, css::PropertyId::MarginLeft, css::ValueId::Auto);
        AddPropertyToPresentationalHintStyle(style, css::PropertyId::MarginRight, 0, css::UnitType::px);
      }
      else
      {
        AddPropertyToPresentationalHintStyle(style, css::PropertyId::MarginLeft, css::ValueId::Auto);
        AddPropertyToPresentationalHintStyle(style, css::PropertyId::MarginRight, css::ValueId::Auto);
      }
      return;
    }
    if (localName == u8"width")
    {
      AddHTMLLengthToStyle(style, css::PropertyId::Width, value);
      return;
    }
    if (localName == u8"color")
    {
      AddPropertyToPresentationalHintStyle(style, css::PropertyId::BorderStyle, css::ValueId::Solid);
      AddHTMLColorToStyle(style, css::PropertyId::BorderColor, value);
      AddHTMLColorToStyle(style, css::PropertyId::BackgroundColor, value);
      return;
    }
    if (localName == u8"noshade")
    {
      if (!HasAttribute(u8"color"))
      {
        AddPropertyToPresentationalHintStyle(style, css::PropertyId::BorderStyle, css::ValueId::Solid);
        auto darkGrayValue =
          css::ValuePool::MainThreadPool().CreateColorValue(boo::Color(boo::Color::darkGray));
        style.SetProperty(css::PropertyId::BorderColor, darkGrayValue);
        style.SetProperty(css::PropertyId::BackgroundColor, krys::move(darkGrayValue));
      }
      return;
    }
    if (localName == u8"size")
    {
      auto size = html::Numbers::ParseInteger(value);
      if (size.Success() && size.Value > 1)
      {
        AddPropertyToPresentationalHintStyle(style, css::PropertyId::Height,
                                             static_cast<double>(size.Value - 2), css::UnitType::px);
      }
      else
      {
        AddPropertyToPresentationalHintStyle(style, css::PropertyId::BorderBottomWidth, 0, css::UnitType::px);
      }
      return;
    }
    HTMLElement::CollectPresentationalHintsForAttribute(localName, namespaceURI, value, style);
  }

#pragma endregion
}
