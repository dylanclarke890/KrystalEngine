#include "Krystal.Booey/HTML/HTMLStyleElement.hpp"
#include "Krystal.Booey/Constants/EventNames.hpp"
#include "Krystal.Booey/CSS/CSSStyleSheet.hpp"
#include "Krystal.Booey/CSS/Queries/MediaQueryParser.hpp"
#include "Krystal.Booey/DOM/Algorithms/ElementAlgorithms.hpp"
#include "Krystal.Booey/DOM/Algorithms/EventDispatcher.hpp"
#include "Krystal.Booey/DOM/Algorithms/TreeQueries.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/HTML/Attributes/Reflection.hpp"
#include "Krystal.Booey/HTML/EventLoop.hpp"
#include "Krystal.Booey/Style/Scope.hpp"

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
    auto *sheet = Sheet();
    return sheet && sheet->Disabled();
  }

  void HTMLStyleElement::Disabled(bool value) noexcept
  {
    if (auto *sheet = Sheet())
    {
      sheet->SetDisabled(value);
    }
  }

  dom::DOMString HTMLStyleElement::Media() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"media").value_or(u8"");
  }

  void HTMLStyleElement::Media(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"media", krys::move(value));
  }

  dom::DOMTokenList &HTMLStyleElement::Blocking() noexcept
  {
    if (!_blocking)
    {
      _blocking = dom::ElementAlgorithms::CreateDOMTokenList(*this, u8"blocking");
    }

    return *_blocking;
  }

#pragma endregion

#pragma region HTMLStyleElement Obsolete members

  dom::DOMString HTMLStyleElement::Type() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"type").value_or(u8"");
  }

  void HTMLStyleElement::Type(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"type", krys::move(value));
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
      _styleSheetOwner.InsertedIntoDocument(*this);
    }
  }

  void HTMLStyleElement::OnRemove(bool isSubtreeRoot, dom::ContainerNode &oldAncestor) noexcept
  {
    HTMLElement::OnRemove(isSubtreeRoot, oldAncestor);
    if (oldAncestor.IsConnected())
    {
      _styleSheetOwner.RemovedFromDocument(*this);
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
    _loadedSheet = !errorOccurred;

    // https://html.spec.whatwg.org/#update-a-style-block: once the sheet and its critical subresources have
    // loaded, queue an element task on the networking task source to fire load (or error) at the element.
    html::EventLoop::Current().QueueTask(
      TaskSource::Networking, [element = ShareRef(*this), loaded = _loadedSheet]
      { dom::EventDispatcher::FireEvent(loaded ? EventNames::Load : EventNames::Error, *element); });
  }

  void HTMLStyleElement::StartLoadingDynamicSheet() noexcept
  {
    _styleSheetOwner.StartLoadingDynamicSheet(*this);
  }

#pragma endregion
}
