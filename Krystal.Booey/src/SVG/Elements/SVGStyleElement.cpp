/*
 * Copyright (C) 2004, 2005 Nikolas Zimmermann <zimmermann@kde.org>
 * Copyright (C) 2004, 2005, 2006, 2007 Rob Buis <buis@kde.org>
 * Copyright (C) 2006, 2013 Apple Inc. All rights reserved.
 * Copyright (C) 2009 Cameron McCormack <cam@mcc.id.au>
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
 */

#include "Krystal.Booey/SVG/Elements/SVGStyleElement.hpp"
#include "Krystal.Booey/CSS/Queries/MediaQueryParser.hpp"
#include "Krystal.Booey/CSS/StyleSheets/CSSStyleSheet.hpp"
#include "Krystal.Booey/DOM/Algorithms/ElementAlgorithms.hpp"
#include "Krystal.Booey/DOM/Algorithms/EventDispatcher.hpp"
#include "Krystal.Booey/DOM/Algorithms/TreeQueries.hpp"
#include "Krystal.Booey/DOM/Events/EventNames.hpp"
#include "Krystal.Booey/DOM/Nodes/Document.hpp"
#include "Krystal.Booey/HTML/EventLoop/EventLoop.hpp"
#include "Krystal.Booey/Style/Scope/Scope.hpp"
#include "Krystal.Booey/SVG/Properties/SVGPropertyOwnerRegistry.hpp"
#include "Krystal.Booey/SVG/SVGNames.hpp"

namespace krys::boo::svg
{
  SVGStyleElement::SVGStyleElement(dom::Document &document) noexcept
      : SVGElement(document, CreateUnique<PropertyRegistry>(*this), SVGElementInterface::Style)
  {
  }

  SVGStyleElement::~SVGStyleElement() noexcept
  {
    _styleSheetOwner.ClearDocumentData(*this);
  }

  bool SVGStyleElement::Disabled() const noexcept
  {
    auto *sheet = Sheet();
    return sheet && sheet->Disabled();
  }

  void SVGStyleElement::Disabled(bool value) noexcept
  {
    if (auto *sheet = Sheet())
    {
      sheet->SetDisabled(value);
    }
  }

  dom::DOMString SVGStyleElement::Type() const noexcept
  {
    return dom::ElementAlgorithms::GetAttributeValue(*this, SVGNames::TypeAttr().LocalName());
  }

  void SVGStyleElement::Type(dom::DOMString value) noexcept
  {
    dom::ElementAlgorithms::SetAttributeValue(*this, SVGNames::TypeAttr().LocalName(), krys::move(value));
  }

  dom::DOMString SVGStyleElement::Media() const noexcept
  {
    return dom::ElementAlgorithms::GetAttributeValue(*this, SVGNames::MediaAttr().LocalName());
  }

  void SVGStyleElement::Media(dom::DOMString value) noexcept
  {
    dom::ElementAlgorithms::SetAttributeValue(*this, SVGNames::MediaAttr().LocalName(), krys::move(value));
  }

  dom::DOMString SVGStyleElement::Title() const noexcept
  {
    return dom::ElementAlgorithms::GetAttributeValue(*this, SVGNames::TitleAttr().LocalName());
  }

  void SVGStyleElement::Title(dom::DOMString value) noexcept
  {
    dom::ElementAlgorithms::SetAttributeValue(*this, SVGNames::TitleAttr().LocalName(), krys::move(value));
  }

  void SVGStyleElement::AttributeChanged(const dom::QualifiedName &name, Maybe<dom::DOMStringView> oldValue,
                                         Maybe<dom::DOMStringView> value) noexcept
  {
    auto newValue = value ? dom::DOMStringAtom(*value) : dom::DOMStringAtom::Null();
    if (name == SVGNames::TitleAttr())
    {
      if (auto *sheet = Sheet(); sheet && !dom::TreeQueries::IsInShadowTree(*this))
      {
        sheet->SetTitle(css::CSSOMString(newValue.View()));
      }
    }
    else if (name == SVGNames::TypeAttr())
    {
      _styleSheetOwner.SetContentType(newValue);
      _styleSheetOwner.ChildrenChanged(*this);
      if (auto *scope = _styleSheetOwner.StyleScope())
      {
        scope->DidChangeStyleSheetContents();
      }
    }
    else if (name == SVGNames::MediaAttr())
    {
      _styleSheetOwner.SetMedia(newValue);
      if (auto *sheet = Sheet())
      {
        sheet->SetMediaQueries(
          css::mq::MediaQueryParser::Parse(newValue.View(), NodeDocument().CssParserContext()));
        if (auto *scope = _styleSheetOwner.StyleScope())
        {
          scope->DidChangeStyleSheetContents();
        }
      }
      else
      {
        _styleSheetOwner.ChildrenChanged(*this);
      }
    }

    SVGElement::AttributeChanged(name, oldValue, value);
  }

  void SVGStyleElement::FinishParsingChildren() noexcept
  {
    _styleSheetOwner.FinishParsingChildren(*this);
    SVGElement::FinishParsingChildren();
  }

  void SVGStyleElement::OnInsert() noexcept
  {
    SVGElement::OnInsert();
    if (IsConnected())
    {
      _styleSheetOwner.InsertedIntoDocument(*this);
    }
  }

  void SVGStyleElement::OnRemove(bool isSubtreeRoot, dom::ContainerNode &oldAncestor) noexcept
  {
    SVGElement::OnRemove(isSubtreeRoot, oldAncestor);
    if (oldAncestor.IsConnected())
    {
      _styleSheetOwner.RemovedFromDocument(*this);
    }
  }

  void SVGStyleElement::OnChildrenChanged() noexcept
  {
    SVGElement::OnChildrenChanged();
    _styleSheetOwner.ChildrenChanged(*this);
  }

  void SVGStyleElement::NotifyLoadedSheetAndAllCriticalSubresources(bool errorOccurred) noexcept
  {
    // As Chrome: an `error` event when a critical subresource (an `@import`) failed to load, and no `load`
    // event.
    if (!errorOccurred)
    {
      return;
    }
    html::EventLoop::Current().QueueTask(html::TaskSource::Networking, NodeDocument(),
                                         [element = ShareRef(*this)]
                                         { dom::EventDispatcher::FireEvent(EventNames::Error, *element); });
  }
}
