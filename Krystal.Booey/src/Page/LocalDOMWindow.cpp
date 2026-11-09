/*
 * Copyright (C) 2006-2025 Apple Inc. All rights reserved.
 * Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies)
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "Krystal.Booey/Page/LocalDOMWindow.hpp"
#include "Krystal.Booey/CSS/CSSOM/CSSComputedStyleDeclaration.hpp"
#include "Krystal.Booey/CSS/Media/MediaQueryList.hpp"
#include "Krystal.Booey/CSS/Selectors/SelectorParser.hpp"
#include "Krystal.Booey/CSS/Selectors/SelectorParserContext.hpp"
#include "Krystal.Booey/DOM/Collections/LiveHTMLCollection.hpp"
#include "Krystal.Booey/DOM/Iterators/SubtreeRanges.hpp"
#include "Krystal.Booey/DOM/Nodes/Document.hpp"
#include "Krystal.Booey/DOM/Nodes/Element.hpp"
#include "Krystal.Booey/HTML/CloseWatcher/CloseWatcherManager.hpp"
#include "Krystal.Booey/HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLElement.hpp"
#include "Krystal.Booey/HTML/EventLoop/EventLoop.hpp"
#include "Krystal.Booey/Page/ChromeClient.hpp"
#include "Krystal.Booey/Page/DOMSelection.hpp"
#include "Krystal.Booey/Page/LocalFrame.hpp"
#include "Krystal.Booey/Page/LocalFrameView.hpp"
#include "Krystal.Booey/Page/Page.hpp"
#include "Krystal.Booey/Page/ScrollBehavior.hpp"
#include "Krystal.Booey/Page/VisualViewport.hpp"
#include <limits>

namespace krys::boo
{
  namespace
  {
    KRYS_NODISCARD List<LocalDOMWindow *> &AllWindows() noexcept
    {
      static List<LocalDOMWindow *> windows;
      return windows;
    }
  }

  LocalDOMWindow::LocalDOMWindow(dom::Document &document) noexcept
      : EventTarget(dom::EventTargetFlags::IsWindow), _document(CreateWeakPtr(&document)),
        _timeOrigin(html::EventLoop::Current().Now()),
        _lastActivationTimestamp(std::numeric_limits<dom::DOMHighResTimeStamp>::infinity()),
        _lastHistoryActionActivationTimestamp(std::numeric_limits<dom::DOMHighResTimeStamp>::infinity())
  {
    krys_assert(document.Frame());
    AllWindows().push_back(this);
  }

  LocalDOMWindow::~LocalDOMWindow() noexcept
  {
    std::erase(AllWindows(), this);
  }

  void LocalDOMWindow::ForEachWindow(const Func<void(LocalDOMWindow &)> &function) noexcept
  {
    auto windows = AllWindows();
    for (auto *window : windows)
    {
      function(*window);
    }
  }

  Ref<LocalDOMWindow> LocalDOMWindow::Create(dom::Document &document) noexcept
  {
    return AdoptRef(*new LocalDOMWindow(document));
  }

  dom::Document *LocalDOMWindow::Document() const noexcept
  {
    return _document.get();
  }

  void LocalDOMWindow::DidAddEventListener(const dom::EventListener &listener) noexcept
  {
    EventTarget::DidAddEventListener(listener);
    if (auto *document = Document())
    {
      document->AddListenerTypeIfNeeded(listener.Type());
    }
  }

  LocalFrame *LocalDOMWindow::Frame() const noexcept
  {
    auto *document = Document();
    return document ? document->Frame() : nullptr;
  }

  Page *LocalDOMWindow::GetPage() const noexcept
  {
    auto *frame = Frame();
    return frame ? frame->GetPage() : nullptr;
  }

  bool LocalDOMWindow::IsCurrentlyDisplayedInFrame() const noexcept
  {
    auto *frame = Frame();
    return frame && frame->Document() == Document();
  }

  html::CustomElementRegistry &LocalDOMWindow::EnsureCustomElementRegistry() noexcept
  {
    if (!_customElementRegistry)
    {
      auto &document = *Document();
      _customElementRegistry = html::CustomElementRegistry::Create(document);
      // A document in a browsing context has its window's registry, unless it was given one.
      if (document.CustomElementRegistry() == nullptr)
      {
        document.SetCustomElementRegistry(RefPtr(_customElementRegistry));
      }
    }
    return *_customElementRegistry;
  }

  void LocalDOMWindow::Focus() noexcept
  {
    // Let current be this's navigable. If current is null, then return.
    auto *page = GetPage();
    if (!page)
    {
      return;
    }
    // If current is a top-level traversable, user agents are encouraged to trigger some sort of notification
    // to indicate to the user that the page is attempting to gain focus: the host decides whether to bring
    // its window to the front. The focusing steps would focus the document's viewport, but the focused
    // element is kept, as browsers do.
    page->GetChromeClient().Focus();
  }

  bool LocalDOMWindow::IsNamedObject(const dom::Element &element, dom::DOMStringView name) noexcept
  {
    if (!Is<html::HTMLElement>(element) || name.empty())
    {
      return false;
    }

    // embed, form, img, or object elements that have a name content attribute whose value is name.
    auto localName = element.LocalName().View();
    if (localName == u8"embed" || localName == u8"form" || localName == u8"img" || localName == u8"object")
    {
      if (auto elementName = element.GetAttribute(u8"name"); elementName && *elementName == name)
      {
        return true;
      }
    }

    // HTML elements that have an id content attribute whose value is name.
    auto id = element.GetAttribute(u8"id");
    return id && *id == name;
  }

  Maybe<Variant<RefPtr<dom::Element>, RefPtr<dom::HTMLCollection>>>
    LocalDOMWindow::NamedItem(dom::DOMStringView name) const noexcept
  {
    // The named objects are in a document tree with window's associated Document as their root. There are no
    // child navigables.
    auto *document = Document();
    if (!document)
    {
      return {};
    }

    RefPtr<dom::Element> first;
    bool several = false;
    for (auto &node : dom::DescendantRange(*document))
    {
      auto *element = DynamicDowncast<dom::Element>(node);
      if (!element || !IsNamedObject(*element, name))
      {
        continue;
      }
      if (first)
      {
        several = true;
        break;
      }
      first = ShareRefPtr(element);
    }

    if (!first)
    {
      return {};
    }

    // If objects has only one element, return that element.
    if (!several)
    {
      return Variant<RefPtr<dom::Element>, RefPtr<dom::HTMLCollection>>(krys::move(first));
    }

    // Otherwise return an HTMLCollection rooted at window's associated Document, whose filter matches only
    // named objects of window with the name name.
    RefPtr<dom::HTMLCollection> collection = CreateRef<dom::LiveHTMLCollection>(
      *document, [name = dom::DOMString(name)](const dom::Element &element)
      { return IsNamedObject(element, name) && element.IsInDocumentTree(); },
      dom::LiveCollectionInvalidation::IdOrName);
    return Variant<RefPtr<dom::Element>, RefPtr<dom::HTMLCollection>>(krys::move(collection));
  }

  List<dom::DOMString> LocalDOMWindow::SupportedPropertyNames() const noexcept
  {
    // The window's document-tree child navigable target name property set (empty: there are no child
    // navigables), then the values of the name content attributes of embed, form, img and object elements,
    // and of the id content attributes of HTML elements, that are non-empty and in the document tree, in tree
    // order, ignoring later duplicates.
    List<dom::DOMString> names;
    auto *document = Document();
    if (!document)
    {
      return names;
    }

    auto add = [&](const Maybe<dom::DOMString> &value)
    {
      if (value && !value->empty() && std::ranges::find(names, *value) == names.end())
      {
        names.push_back(*value);
      }
    };

    for (auto &node : dom::DescendantRange(*document))
    {
      auto *element = DynamicDowncast<dom::Element>(node);
      if (!element || !Is<html::HTMLElement>(*element))
      {
        continue;
      }
      auto localName = element->LocalName().View();
      if (localName == u8"embed" || localName == u8"form" || localName == u8"img" || localName == u8"object")
      {
        add(element->GetAttribute(u8"name"));
      }
      add(element->GetAttribute(u8"id"));
    }
    return names;
  }

  html::CloseWatcherManager &LocalDOMWindow::GetCloseWatcherManager() const noexcept
  {
    if (!_closeWatcherManager)
    {
      _closeWatcherManager = html::CloseWatcherManager::Create();
    }
    return *_closeWatcherManager;
  }

  html::CloseWatcherManager *LocalDOMWindow::CloseWatcherManagerIfExists() const noexcept
  {
    return _closeWatcherManager.get();
  }

  RefPtr<css::MediaQueryList> LocalDOMWindow::MatchMedia(dom::DOMStringView query) noexcept
  {
    auto *document = Document();
    return document ? RefPtr<css::MediaQueryList>(document->GetMediaQueryMatcher().MatchMedia(query))
                    : nullptr;
  }

  VisualViewport &LocalDOMWindow::GetVisualViewport() const noexcept
  {
    if (!_visualViewport)
    {
      _visualViewport = VisualViewport::Create(const_cast<LocalDOMWindow &>(*this));
      _visualViewport->Update();
    }
    return *_visualViewport;
  }

  VisualViewport *LocalDOMWindow::OptionalVisualViewport() const noexcept
  {
    return _visualViewport.get();
  }

  int32 LocalDOMWindow::InnerWidth() const noexcept
  {
    auto *frame = Frame();
    auto *view = frame ? frame->View() : nullptr;
    if (!view)
    {
      return 0;
    }
    return view->MapFromLayoutToCSSUnits(
      view->UnobscuredContentRect(ScrollableArea::VisibleContentRectIncludesScrollbars::Yes).Width());
  }

  int32 LocalDOMWindow::InnerHeight() const noexcept
  {
    auto *frame = Frame();
    auto *view = frame ? frame->View() : nullptr;
    if (!view)
    {
      return 0;
    }
    return view->MapFromLayoutToCSSUnits(
      view->UnobscuredContentRect(ScrollableArea::VisibleContentRectIncludesScrollbars::Yes).Height());
  }

  int32 LocalDOMWindow::ScrollX() const noexcept
  {
    auto *frame = Frame();
    auto *view = frame ? frame->View() : nullptr;
    if (!view || !view->ContentsScrollPosition().X())
    {
      return 0;
    }

    (void)frame->Document()->UpdateLayoutIgnorePendingStylesheets();

    // Layout may have affected the current frame.
    auto *frameAfterLayout = Frame();
    auto *viewAfterLayout = frameAfterLayout ? frameAfterLayout->View() : nullptr;
    return viewAfterLayout
             ? viewAfterLayout->MapFromLayoutToCSSUnits(viewAfterLayout->ContentsScrollPosition().X())
             : 0;
  }

  int32 LocalDOMWindow::ScrollY() const noexcept
  {
    auto *frame = Frame();
    auto *view = frame ? frame->View() : nullptr;
    if (!view || !view->ContentsScrollPosition().Y())
    {
      return 0;
    }

    (void)frame->Document()->UpdateLayoutIgnorePendingStylesheets();

    // Layout may have affected the current frame.
    auto *frameAfterLayout = Frame();
    auto *viewAfterLayout = frameAfterLayout ? frameAfterLayout->View() : nullptr;
    return viewAfterLayout
             ? viewAfterLayout->MapFromLayoutToCSSUnits(viewAfterLayout->ContentsScrollPosition().Y())
             : 0;
  }

  void LocalDOMWindow::ScrollTo(double x, double y) const noexcept
  {
    ScrollTo(ScrollToOptions(x, y));
  }

  void LocalDOMWindow::ScrollTo(const ScrollToOptions &options) const noexcept
  {
    ScrollTo(options, ScrollSnapPointSelectionMethod::Closest);
  }

  void LocalDOMWindow::ScrollTo(const ScrollToOptions &options,
                                ScrollSnapPointSelectionMethod snapPointSelectionMethod) const noexcept
  {
    if (!IsCurrentlyDisplayedInFrame())
    {
      return;
    }

    RefPtr<LocalFrameView> view = ShareRefPtr(Frame()->View());
    if (!view)
    {
      return;
    }

    ScrollToOptions scrollToOptions = NormalizeNonFiniteCoordinatesOrFallBackTo(
      options, view->ContentsScrollPosition().X(), view->ContentsScrollPosition().Y());

    // This is an optimization for the common case of scrolling to (0, 0) when the scroller is already at the
    // origin. If an animated scroll is in progress, this optimization is skipped to ensure that the animated
    // scroll is really stopped.
    if (view->GetScrollAnimationStatus() == ScrollAnimationStatus::NotAnimating && !*scrollToOptions.Left
        && !*scrollToOptions.Top && view->ContentsScrollPosition().IsZero())
    {
      return;
    }

    view->CancelScheduledScrolls();
    (void)Document()->UpdateLayoutIgnorePendingStylesheets();

    IntPoint layoutPos(view->MapFromCSSToLayoutUnits(static_cast<int32>(*scrollToOptions.Left)),
                       view->MapFromCSSToLayoutUnits(static_cast<int32>(*scrollToOptions.Top)));

    auto animated = UseSmoothScrolling(scrollToOptions.Behavior.value_or(ScrollBehavior::Auto),
                                       Document()->DocumentElement().get())
                      ? ScrollIsAnimated::Yes
                      : ScrollIsAnimated::No;
    view->SetContentsScrollPosition(layoutPos, ScrollPositionChangeOptions::CreateProgrammaticWithOptions(
                                                 animated, snapPointSelectionMethod));
  }

  void LocalDOMWindow::ScrollBy(double x, double y) const noexcept
  {
    ScrollBy(ScrollToOptions(x, y));
  }

  void LocalDOMWindow::ScrollBy(const ScrollToOptions &options) const noexcept
  {
    if (!IsCurrentlyDisplayedInFrame())
    {
      return;
    }

    (void)Document()->UpdateLayoutIgnorePendingStylesheets();

    auto *view = Frame() ? Frame()->View() : nullptr;
    if (!view)
    {
      return;
    }

    ScrollToOptions scrollToOptions = NormalizeNonFiniteCoordinatesOrFallBackTo(options, 0, 0);
    *scrollToOptions.Left += view->MapFromLayoutToCSSUnits(view->ContentsScrollPosition().X());
    *scrollToOptions.Top += view->MapFromLayoutToCSSUnits(view->ContentsScrollPosition().Y());
    ScrollTo(scrollToOptions, ScrollSnapPointSelectionMethod::Directional);
  }

  double LocalDOMWindow::DevicePixelRatio() const noexcept
  {
    auto *frame = Frame();
    if (!frame)
    {
      return 0;
    }
    // Page zoom applies to all frames synchronously, so include it in devicePixelRatio
    // for both main frame and iframes. FrameScaleFactor() doesn't include page zoom
    // since page zoom is applied globally for rendering.
    return frame->GetMediaEnvironment().DeviceScaleFactor * frame->FrameScaleFactor()
           * frame->PageZoomFactor();
  }

  Ref<css::CSSStyleDeclaration>
    LocalDOMWindow::GetComputedStyle(dom::Element &element,
                                     const css::CSSOMString &pseudoElement) const noexcept
  {
    if (!pseudoElement.starts_with(u8':'))
    {
      return css::CSSComputedStyleDeclaration::Create(element, null);
    }

    // FIXME: This does not work for pseudo-elements that take arguments (webkit.org/b/264103).
    auto [pseudoElementIsParsable, pseudoElementIdentifier] = css::SelectorParser::ParsePseudoElement(
      pseudoElement, css::SelectorParserContext {element.NodeDocument()});
    if (!pseudoElementIsParsable)
    {
      return css::CSSComputedStyleDeclaration::CreateEmpty(element);
    }
    return css::CSSComputedStyleDeclaration::Create(element, pseudoElementIdentifier);
  }

  namespace
  {

  }

  // https://html.spec.whatwg.org/#activation-notification
  void LocalDOMWindow::NotifyActivated() noexcept
  {
    // 1. Assert: document is fully active.
    // 2. Let windows be « document's relevant global object ».
    // 3. Extend windows with the active window of each of document's ancestor navigables.
    // 4. Extend windows with the active window of each of document's descendant navigables, filtered to
    //    include only those navigables whose active document's origin is same origin with document's origin.
    // (There are no ancestor or descendant frames, so windows is this window.)
    // 5. For each window in windows:
    // 5.1. Set window's last activation timestamp to the current high resolution time.
    _lastActivationTimestamp = NowTimestamp();
    // 5.2. Notify the close watcher manager about user activation given window.
    GetCloseWatcherManager().NotifyAboutUserActivation();
  }

  bool LocalDOMWindow::HasStickyActivation() const noexcept
  {
    // When the current high resolution time given W is greater than or equal to the last activation timestamp
    // in W, W is said to have sticky activation.
    return NowTimestamp() >= _lastActivationTimestamp;
  }

  bool LocalDOMWindow::HasTransientActivation() const noexcept
  {
    // When the current high resolution time is greater than or equal to the last activation timestamp in W,
    // and less than the last activation timestamp in W plus the transient activation duration, then W is said
    // to have transient activation.
    auto now = NowTimestamp();
    return now >= _lastActivationTimestamp && now < _lastActivationTimestamp + TransientActivationDuration();
  }

  bool LocalDOMWindow::HasHistoryActionActivation() const noexcept
  {
    // When the last history-action activation timestamp of W is not equal to the last activation timestamp of
    // W, then W is said to have history-action activation.
    return _lastHistoryActionActivationTimestamp != _lastActivationTimestamp;
  }

  // https://html.spec.whatwg.org/#consume-user-activation
  bool LocalDOMWindow::ConsumeTransientActivation() noexcept
  {
    // (Without transient activation, consuming changes nothing observable, so it returns early to report it.)
    if (!HasTransientActivation())
    {
      return false;
    }
    // 1. If W's navigable is null, then return.
    // 2. Let top be W's navigable's top-level traversable.
    // 3. Let navigables be the inclusive descendant navigables of top's active document.
    // 4. Let windows be the list of Window objects constructed by taking the active window of each item in
    //    navigables.
    // (There are no other frames, so windows is this window.)
    // 5. For each window in windows, if window's last activation timestamp is not positive infinity, then set
    //    window's last activation timestamp to negative infinity.
    if (_lastActivationTimestamp != std::numeric_limits<dom::DOMHighResTimeStamp>::infinity())
    {
      _lastActivationTimestamp = -std::numeric_limits<dom::DOMHighResTimeStamp>::infinity();
    }
    return true;
  }

  // https://html.spec.whatwg.org/#consume-history-action-user-activation
  bool LocalDOMWindow::ConsumeHistoryActionUserActivation() noexcept
  {
    if (!HasHistoryActionActivation())
    {
      return false;
    }
    // 1. If W's navigable is null, then return.
    // 2. Let top be W's navigable's top-level traversable.
    // 3. Let navigables be the inclusive descendant navigables of top's active document.
    // 4. Let windows be the list of Window objects constructed by taking the active window of each item in
    //    navigables.
    // (There are no other frames, so windows is this window.)
    // 5. For each window in windows, set window's last history-action activation timestamp to window's last
    //    activation timestamp.
    _lastHistoryActionActivationTimestamp = _lastActivationTimestamp;
    return true;
  }

  // https://html.spec.whatwg.org/#dom-queuemicrotask
  void LocalDOMWindow::QueueMicrotask(Func<void()> &&callback) noexcept
  {
    // Queue a microtask to invoke callback with « » and "report".
    // (C++ callbacks don't throw.)
    html::EventLoop::Current().QueueMicrotask(krys::move(callback));
  }

  int32 LocalDOMWindow::RequestAnimationFrame(dom::FrameRequestCallback &&callback) noexcept
  {
    auto *document = Document();
    return document ? document->RequestAnimationFrame(krys::move(callback)) : 0;
  }

  void LocalDOMWindow::CancelAnimationFrame(int32 handle) noexcept
  {
    if (auto *document = Document())
    {
      document->CancelAnimationFrame(handle);
    }
  }

  dom::DOMHighResTimeStamp LocalDOMWindow::NowTimestamp() const noexcept
  {
    return std::chrono::duration<double, std::milli>(html::EventLoop::Current().Now() - _timeOrigin).count();
  }

  void LocalDOMWindow::FreezeNowTimestamp() noexcept
  {
    _frozenNowTimestamp = NowTimestamp();
  }

  void LocalDOMWindow::UnfreezeNowTimestamp() noexcept
  {
    _frozenNowTimestamp = null;
  }

  dom::DOMHighResTimeStamp LocalDOMWindow::FrozenNowTimestamp() const noexcept
  {
    return _frozenNowTimestamp.value_or(NowTimestamp());
  }

  DOMSelection *LocalDOMWindow::GetSelection() const noexcept
  {
    // Return the associated Document's selection: the same object as document.getSelection(), which is null
    // without a browsing context.
    if (!IsCurrentlyDisplayedInFrame())
    {
      return nullptr;
    }
    return Document()->GetSelection();
  }
}
