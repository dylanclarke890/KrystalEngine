#include "Krystal.Booey/HTML/Elements/HTMLStyleElement.hpp"
#include "Krystal.Booey/CSS/CSSOM/CSSStyleSheet.hpp"
#include "Krystal.Booey/CSS/Queries/MediaQuery.hpp"
#include "Krystal.Booey/CSS/Queries/MediaQueryEvaluator.hpp"
#include "Krystal.Booey/CSS/Queries/MediaQueryParser.hpp"
#include "Krystal.Booey/DOM/Algorithms/ElementAlgorithms.hpp"
#include "Krystal.Booey/DOM/Algorithms/EventDispatcher.hpp"
#include "Krystal.Booey/DOM/Algorithms/TreeQueries.hpp"
#include "Krystal.Booey/DOM/Collections/DOMTokenList.hpp"
#include "Krystal.Booey/DOM/Events/EventNames.hpp"
#include "Krystal.Booey/DOM/Nodes/Document.hpp"
#include "Krystal.Booey/HTML/Algorithms/RenderBlocking.hpp"
#include "Krystal.Booey/HTML/Attributes/ReflectAttribute.hpp"
#include "Krystal.Booey/HTML/EventLoop/EventLoop.hpp"
#include "Krystal.Booey/Page/FrameView.hpp"
#include "Krystal.Booey/Style/ResolveForDocument.hpp"
#include "Krystal.Booey/Style/Scope/Scope.hpp"
#include "Krystal.Core/Strings/ASCII.hpp"

namespace krys::boo
{
  HTMLStyleElement::HTMLStyleElement(Document &document) noexcept
      : HTMLElement(document, html::HTMLElementInterface::Style)
  {
  }

  HTMLStyleElement::~HTMLStyleElement() noexcept
  {
    _styleSheetOwner.ClearDocumentData(*this);
  }

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

  DOMString HTMLStyleElement::Media() const noexcept
  {
    return html::Reflect<DOMString>(*this, u8"media").value_or(u8"");
  }

  void HTMLStyleElement::Media(DOMString value) noexcept
  {
    html::Reflect<DOMString>(*this, u8"media", krys::move(value));
  }

  DOMTokenList &HTMLStyleElement::Blocking() noexcept
  {
    if (!_blocking)
    {
      // `render` is the only supported blocking token.
      _blocking = DOMTokenList::Create(*this, u8"blocking", [](Document &, DOMStringView token)
                                       { return text::IsASCIICaselessEqual(token, u8"render"); });
    }
    return *_blocking;
  }

#pragma endregion

#pragma region HTMLStyleElement Obsolete members

  DOMString HTMLStyleElement::Type() const noexcept
  {
    return html::Reflect<DOMString>(*this, u8"type").value_or(u8"");
  }

  void HTMLStyleElement::Type(DOMString value) noexcept
  {
    html::Reflect<DOMString>(*this, u8"type", krys::move(value));
  }

#pragma endregion

#pragma region LinkStyle

  CSSStyleSheet *HTMLStyleElement::Sheet() const noexcept
  {
    return _styleSheetOwner.Sheet();
  }

#pragma endregion

#pragma region Extension Hooks

  void HTMLStyleElement::OnAttributeChanged(const DOMStringAtom &localName, Maybe<DOMStringView> oldValue,
                                            Maybe<DOMStringView> value,
                                            const DOMStringAtom &namespaceURI) noexcept
  {
    HTMLElement::OnAttributeChanged(localName, oldValue, value, namespaceURI);
    if (namespaceURI != null)
    {
      return;
    }

    if (localName == u8"blocking")
    {
      if (_blocking)
      {
        _blocking->AssociatedAttributeValueChanged();
      }
      // An element that's no longer potentially render-blocking stops blocking rendering.
      if (!html::RenderBlocking::HasRenderBlockingToken(*this) && !_createdByParser)
      {
        UnblockRendering();
      }
      return;
    }

    auto newValue = value ? DOMStringAtom(*value) : null;
    if (localName == u8"title")
    {
      // Only a sheet in a document tree takes the element's title.
      if (auto *sheet = Sheet(); sheet && !dom::TreeQueries::IsInShadowTree(*this))
      {
        sheet->SetTitle(newValue.ToString());
      }
    }
    else if (localName == u8"media")
    {
      _styleSheetOwner.SetMedia(newValue);
      // An existing sheet only needs its media queries swapped; otherwise the update runs again.
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
      // The type decides whether there's a sheet at all.
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
      // The parser connects an element before adding its children, so one with children still to come
      // is the parser's.
      _createdByParser = !IsFinishedParsingChildren();
      _styleSheetOwner.InsertedIntoDocument(*this);
    }
  }

  void HTMLStyleElement::OnRemove(bool isSubtreeRoot, ContainerNode &oldAncestor) noexcept
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
    // https://html.spec.whatwg.org/multipage/semantics.html#update-a-style-block
    // 1. Let element be the style element associated with the style sheet in question.
    // 2. Let success be true.
    // 3. If the attempts to obtain any of the style sheet's critical subresources failed for any reason, set
    //    success to false.
    const bool success = !errorOccurred;

    // 4. Queue an element task on the networking task source given element and the following steps:
    // 4a. If success is true, fire an event named load at element.
    // 4b. Otherwise, fire an event named error at element.
    // 4c. If element contributes a script-blocking style sheet: (Skipped: nothing waits for scripts.)
    // 4d. Unblock rendering on element.
    // (Done now rather than in the task, so a sheet with nothing to load, which is processed as it's
    // parsed, never holds rendering back; see DIFFERENCES.md.)
    UnblockRendering();
    html::EventLoop::Current().QueueTask(
      html::TaskSource::Networking, NodeDocument(), [element = ShareRef(*this), success]
      { dom::EventDispatcher::FireEvent(success ? event_names::Load : event_names::Error, *element); });
  }

  // https://html.spec.whatwg.org/multipage/urls-and-fetching.html#potentially-render-blocking
  void HTMLStyleElement::PotentiallyBlockRendering(const css::mq::MediaQueryList &mediaQueries) noexcept
  {
    // An element is potentially render-blocking if its blocking tokens set contains "render", or if it is
    // implicitly potentially render-blocking, as a style element is when the parser made it.
    if (!html::RenderBlocking::HasRenderBlockingToken(*this) && !_createdByParser)
    {
      return;
    }

    // Its media must match the environment, which is resolved against the document's style when it has a
    // render tree.
    auto &document = NodeDocument();
    if (!mediaQueries.empty())
    {
      Maybe<RenderStyle> documentStyle;
      if (document.HasLivingRenderTree())
      {
        documentStyle = style::ResolveForDocument(document);
      }
      auto *frameView = document.View();
      css::mq::MediaQueryEvaluator evaluator(frameView ? frameView->MediaType() : CSSOMStringAtom(u8"screen"),
                                             document, documentStyle ? &*documentStyle : nullptr);
      if (!evaluator.Evaluate(mediaQueries))
      {
        return;
      }
    }

    html::RenderBlocking::BlockRendering(*this);
    _isRenderBlocking = true;
  }

  void HTMLStyleElement::UnblockRendering() noexcept
  {
    if (_isRenderBlocking)
    {
      html::RenderBlocking::UnblockRendering(*this);
      _isRenderBlocking = false;
    }
  }

  void HTMLStyleElement::StartLoadingDynamicSheet() noexcept
  {
    _styleSheetOwner.StartLoadingDynamicSheet(*this);
  }

#pragma endregion
}
