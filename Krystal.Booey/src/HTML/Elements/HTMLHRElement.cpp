/*
 * Copyright (C) 1999 Lars Knoll (knoll@kde.org)
 *           (C) 1999 Antti Koivisto (koivisto@kde.org)
 * Copyright (C) 2003-2025 Apple Inc. All rights reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 */

#include "Krystal.Booey/HTML/Elements/HTMLHRElement.hpp"
#include "Krystal.Booey/CSS/Values/ColorValue.hpp"
#include "Krystal.Booey/HTML/Attributes/ReflectAttribute.hpp"
#include "Krystal.Booey/HTML/Internals/PresentationalHints.hpp"
#include "Krystal.Booey/HTML/MicroParsers/Numbers.hpp"
#include "Krystal.Booey/Infra/StringAlgorithms.hpp"

namespace krys::boo::html
{
  HTMLHRElement::HTMLHRElement(dom::Document &document) noexcept
      : HTMLElement(document, HTMLElementInterface::HR)
  {
  }

#pragma region HTMLHRElement Obsolete members

  dom::DOMString HTMLHRElement::Align() const noexcept
  {
    return Reflect<dom::DOMString>(*this, u8"align").value_or(u8"");
  }

  void HTMLHRElement::Align(dom::DOMString value) noexcept
  {
    Reflect<dom::DOMString>(*this, u8"align", krys::move(value));
  }

  dom::DOMString HTMLHRElement::Color() const noexcept
  {
    return Reflect<dom::DOMString>(*this, u8"color").value_or(u8"");
  }

  void HTMLHRElement::Color(dom::DOMString value) noexcept
  {
    Reflect<dom::DOMString>(*this, u8"color", krys::move(value));
  }

  bool HTMLHRElement::NoShade() const noexcept
  {
    return Reflect<bool>(*this, u8"noshade");
  }

  void HTMLHRElement::NoShade(bool value) noexcept
  {
    Reflect<bool>(*this, u8"noshade", krys::move(value));
  }

  dom::DOMString HTMLHRElement::Size() const noexcept
  {
    return Reflect<dom::DOMString>(*this, u8"size").value_or(u8"");
  }

  void HTMLHRElement::Size(dom::DOMString value) noexcept
  {
    Reflect<dom::DOMString>(*this, u8"size", krys::move(value));
  }

  dom::DOMString HTMLHRElement::Width() const noexcept
  {
    return Reflect<dom::DOMString>(*this, u8"width").value_or(u8"");
  }

  void HTMLHRElement::Width(dom::DOMString value) noexcept
  {
    Reflect<dom::DOMString>(*this, u8"width", krys::move(value));
  }

#pragma endregion

#pragma region Presentational hints

  bool HTMLHRElement::HasPresentationalHintsForAttribute(dom::DOMStringAtom localName,
                                                         dom::DOMStringAtom namespaceURI) const noexcept
  {
    if (namespaceURI == dom::DOMStringAtom::Null()
        && (localName == u8"width" || localName == u8"color" || localName == u8"noshade"
            || localName == u8"size"))
    {
      return true;
    }
    return HTMLElement::HasPresentationalHintsForAttribute(localName, namespaceURI);
  }

  void HTMLHRElement::CollectPresentationalHintsForAttribute(dom::DOMStringAtom localName,
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
      auto size = Numbers::ParseInteger(value);
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
