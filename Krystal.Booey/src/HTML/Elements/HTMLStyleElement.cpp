/*
 * Copyright (C) 1999 Lars Knoll (knoll@kde.org)
 *           (C) 1999 Antti Koivisto (koivisto@kde.org)
 *           (C) 2001 Dirk Mueller (mueller@kde.org)
 * Copyright (C) 2003, 2010, 2013 Apple Inc. All rights reserved.
 *           (C) 2007 Rob Buis (buis@kde.org)
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

#include "Krystal.Booey/HTML/Elements/HTMLStyleElement.hpp"
#include "Krystal.Booey/CSS/Queries/MediaQueryEvaluator.hpp"
#include "Krystal.Booey/CSS/Queries/MediaQueryParser.hpp"
#include "Krystal.Booey/CSS/StyleSheets/CSSStyleSheet.hpp"
#include "Krystal.Booey/DOM/Algorithms/ElementAlgorithms.hpp"
#include "Krystal.Booey/DOM/Algorithms/EventDispatcher.hpp"
#include "Krystal.Booey/DOM/Algorithms/TreeQueries.hpp"
#include "Krystal.Booey/DOM/Events/EventNames.hpp"
#include "Krystal.Booey/DOM/Nodes/Document.hpp"
#include "Krystal.Booey/HTML/Algorithms/RenderBlocking.hpp"
#include "Krystal.Booey/HTML/Attributes/ReflectAttribute.hpp"
#include "Krystal.Booey/HTML/EventLoop/EventLoop.hpp"
#include "Krystal.Booey/Page/LocalFrameView.hpp"
#include "Krystal.Booey/Style/ResolveForDocument.hpp"
#include "Krystal.Booey/Style/Scope/Scope.hpp"

#include "Krystal.Core/Text/ASCII.hpp"

namespace krys::boo::html
{
  HTMLStyleElement::HTMLStyleElement(dom::Document &document) noexcept
      : HTMLElement(document, HTMLElementInterface::Style)
  {
  }

  HTMLStyleElement::~HTMLStyleElement() noexcept
  {
    _styleSheetOwner.ClearDocumentData(*this);
  }

#pragma region LinkStyle

  css::CSSStyleSheet *HTMLStyleElement::Sheet() const noexcept
  {
    return _styleSheetOwner.Sheet();
  }

#pragma endregion

#pragma region HTMLStyleElement

  bool HTMLStyleElement::Disabled() const noexcept
  {
    // 1. If this does not have an associated CSS style sheet, return false.
    // 2. If this's associated CSS style sheet's disabled flag is set, return true.
    // 3. Return false.
    auto *sheet = Sheet();
    return sheet && sheet->Disabled();
  }

  void HTMLStyleElement::Disabled(bool value) noexcept
  {
    // 1. If this does not have an associated CSS style sheet, return.
    // 2. If the given value is true, set this's associated CSS style sheet's disabled flag. Otherwise, unset
    //    this's associated CSS style sheet's disabled flag.
    if (auto *sheet = Sheet())
    {
      sheet->SetDisabled(value);
    }
  }

  dom::DOMString HTMLStyleElement::Media() const noexcept
  {
    return Reflect<dom::DOMString>(*this, u8"media").value_or(u8"");
  }

  void HTMLStyleElement::Media(dom::DOMString value) noexcept
  {
    Reflect<dom::DOMString>(*this, u8"media", krys::move(value));
  }

  dom::DOMTokenList &HTMLStyleElement::Blocking() noexcept
  {
    if (!_blocking)
    {
      _blocking = dom::ElementAlgorithms::CreateDOMTokenList(
        *this, u8"blocking", [](dom::Document &, dom::DOMStringView token)
        { return text::IsASCIICaselessEqual(token, u8"render"); });
    }

    return *_blocking;
  }

#pragma endregion

#pragma region HTMLStyleElement Obsolete members

  dom::DOMString HTMLStyleElement::Type() const noexcept
  {
    return Reflect<dom::DOMString>(*this, u8"type").value_or(u8"");
  }

  void HTMLStyleElement::Type(dom::DOMString value) noexcept
  {
    Reflect<dom::DOMString>(*this, u8"type", krys::move(value));
  }

#pragma endregion

#pragma region Extension Hooks

  void HTMLStyleElement::OnAttributeChanged(dom::DOMStringAtom localName, Maybe<dom::DOMStringView> oldValue,
                                            Maybe<dom::DOMStringView> value,
                                            dom::DOMStringAtom namespaceURI) noexcept
  {
    HTMLElement::OnAttributeChanged(localName, oldValue, value, namespaceURI);
    if (namespaceURI != dom::DOMStringAtom::Null())
    {
      return;
    }

    // The token lists follow their attributes.
    if (localName == u8"blocking" && _blocking)
    {
      _blocking->AssociatedAttributeValueChanged();
    }
    // An element that's no longer potentially render-blocking stops blocking rendering.
    if (localName == u8"blocking" && !RenderBlocking::HasRenderBlockingToken(*this) && !_createdByParser)
    {
      UnblockRendering();
    }

    auto newValue = value ? dom::DOMStringAtom(*value) : dom::DOMStringAtom::Null();
    if (localName == u8"title")
    {
      if (auto *sheet = Sheet(); sheet && !dom::TreeQueries::IsInShadowTree(*this))
      {
        sheet->SetTitle(css::CSSOMString(newValue.View()));
      }
    }
    else if (localName == u8"media")
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
    else if (localName == u8"type")
    {
      _styleSheetOwner.SetContentType(newValue);
      _styleSheetOwner.ChildrenChanged(*this);
      if (auto *scope = _styleSheetOwner.StyleScope())
      {
        scope->DidChangeStyleSheetContents();
      }
    }
  }

  void HTMLStyleElement::OnInsert() noexcept
  {
    HTMLElement::OnInsert();
    if (IsConnected())
    {
      // The parser inserts the elements it makes before their children.
      _createdByParser = !IsFinishedParsingChildren();
      _styleSheetOwner.InsertedIntoDocument(*this);
    }
  }

  void HTMLStyleElement::OnRemove(bool isSubtreeRoot, dom::ContainerNode &oldAncestor) noexcept
  {
    HTMLElement::OnRemove(isSubtreeRoot, oldAncestor);
    if (oldAncestor.IsConnected())
    {
      _styleSheetOwner.RemovedFromDocument(*this);
      UnblockRendering();
    }
  }

  void HTMLStyleElement::OnChildrenChanged() noexcept
  {
    HTMLElement::OnChildrenChanged();
    _styleSheetOwner.ChildrenChanged(*this);
  }

  void HTMLStyleElement::FinishParsingChildren() noexcept
  {
    _styleSheetOwner.FinishParsingChildren(*this);
    HTMLElement::FinishParsingChildren();
  }

#pragma endregion

#pragma region Style Sheet Owner

  bool HTMLStyleElement::SheetLoaded() noexcept
  {
    return _styleSheetOwner.SheetLoaded(*this);
  }

  void HTMLStyleElement::NotifyLoadedSheetAndAllCriticalSubresources(bool errorOccurred) noexcept
  {
    // Once the attempts to obtain the style sheet's critical subresources, if any, are complete, or, if the
    // style sheet has no critical subresources, once the style sheet has been parsed and processed:
    // https://html.spec.whatwg.org/#update-a-style-block
    // 1. Let element be the style element associated with the style sheet in question.
    // 2. Let success be true.
    // 3. If the attempts to obtain any of the style sheet's critical subresources failed for any reason, set
    //    success to false.
    _loadedSheet = !errorOccurred;

    // 4. Queue an element task on the networking task source given element and the following steps:
    // 4a. If success is true, fire an event named load at element.
    // 4b. Otherwise, fire an event named error at element.
    // 4c. If element contributes a script-blocking style sheet: (Skipped: nothing waits for scripts.)
    // 4d. Unblock rendering on element.
    // (Unblocked now rather than in the task, so a sheet without critical subresources, which is processed as
    // it's parsed, never holds back rendering; see DIFFERENCES.md.)
    UnblockRendering();
    html::EventLoop::Current().QueueTask(
      TaskSource::Networking, NodeDocument(), [element = ShareRef(*this), loaded = _loadedSheet]
      { dom::EventDispatcher::FireEvent(loaded ? EventNames::Load : EventNames::Error, *element); });
  }

  void HTMLStyleElement::PotentiallyBlockRendering(const css::mq::MediaQueryList &mediaQueries) noexcept
  {
    // An element is potentially render-blocking if its blocking tokens set contains "render", or if it is
    // implicitly potentially render-blocking: a style element is if the parser made it.
    // https://html.spec.whatwg.org/#potentially-render-blocking
    if (!RenderBlocking::HasRenderBlockingToken(*this) && !_createdByParser)
    {
      return;
    }

    auto &document = NodeDocument();
    if (!mediaQueries.empty())
    {
      Maybe<RenderStyle> documentStyle;
      if (document.HasLivingRenderTree())
      {
        documentStyle = style::ResolveForDocument(document);
      }
      auto *frameView = document.View();
      css::mq::MediaQueryEvaluator evaluator(frameView ? frameView->MediaType()
                                                       : css::CSSOMStringAtom(u8"screen"),
                                             document, documentStyle ? &*documentStyle : nullptr);
      if (!evaluator.Evaluate(mediaQueries))
      {
        return;
      }
    }

    RenderBlocking::BlockRendering(*this);
    _isRenderBlocking = true;
  }

  void HTMLStyleElement::UnblockRendering() noexcept
  {
    if (_isRenderBlocking)
    {
      RenderBlocking::UnblockRendering(*this);
      _isRenderBlocking = false;
    }
  }

  void HTMLStyleElement::StartLoadingDynamicSheet() noexcept
  {
    _styleSheetOwner.StartLoadingDynamicSheet(*this);
  }

#pragma endregion
}
