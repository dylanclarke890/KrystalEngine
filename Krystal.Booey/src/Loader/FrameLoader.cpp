/*
 * Copyright (C) 2006-2025 Apple Inc. All rights reserved.
 * Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies)
 * Copyright (C) 2008, 2009 Torch Mobile Inc. All rights reserved. (http://www.torchmobile.com/)
 * Copyright (C) 2008 Alp Toker <alp@atoker.com>
 * Copyright (C) Research In Motion Limited 2009. All rights reserved.
 * Copyright (C) 2011 Kris Jordan <krisjordan@gmail.com>
 * Copyright (C) 2011 Google Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1.  Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 * 2.  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 * 3.  Neither the name of Apple Inc. ("Apple") nor the names of
 *     its contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE AND ITS CONTRIBUTORS "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL APPLE OR ITS CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "Krystal.Booey/Loader/FrameLoader.hpp"
#include "Krystal.Booey/Dom/Algorithms/DocumentAlgorithms.hpp"
#include "Krystal.Booey/Dom/Algorithms/EventDispatcher.hpp"
#include "Krystal.Booey/Dom/Events/EventNames.hpp"
#include "Krystal.Booey/Dom/Nodes/Document.hpp"
#include "Krystal.Booey/Dom/Nodes/Element.hpp"
#include "Krystal.Booey/Html/Elements/HTMLFormElement.hpp"
#include "Krystal.Booey/Html/EventLoop/EventLoop.hpp"
#include "Krystal.Booey/Html/Window.hpp"
#include "Krystal.Booey/Input/EventHandler.hpp"
#include "Krystal.Booey/Loader/Cache/CachedResourceLoader.hpp"
#include "Krystal.Booey/Loader/DocumentLoader.hpp"
#include "Krystal.Booey/Loader/FormSubmission.hpp"
#include "Krystal.Booey/Loader/FrameLoadRequest.hpp"
#include "Krystal.Booey/Loader/NavigationAction.hpp"
#include "Krystal.Booey/Page/Clients/BooeyClient.hpp"
#include "Krystal.Booey/Page/Frame.hpp"
#include "Krystal.Booey/Page/FrameView.hpp"
#include "Krystal.Core/Strings/ASCII.hpp"
#include "Krystal.Core/Utils/SetForScope.hpp"

namespace krys::boo
{
  FrameLoader::FrameLoader(boo::Frame &frame, UniquePtr<FrameLoaderClient> &&client) noexcept
      : _frame(frame), _client(client ? krys::move(client) : CreateUnique<EmptyFrameLoaderClient>())
  {
  }

  FrameLoader::~FrameLoader() noexcept
  {
    if (_provisionalDocumentLoader)
    {
      _provisionalDocumentLoader->DetachFromFrame();
    }
    if (_documentLoader)
    {
      _documentLoader->DetachFromFrame();
    }
  }

  DocumentLoader *FrameLoader::GetDocumentLoader() const noexcept
  {
    return _documentLoader.get();
  }

  DocumentLoader *FrameLoader::ProvisionalDocumentLoader() const noexcept
  {
    return _provisionalDocumentLoader.get();
  }

  bool FrameLoader::IsLoading() const noexcept
  {
    return _provisionalDocumentLoader || (_documentLoader && _documentLoader->IsLoadingMainResource())
           || _state != FrameState::Complete;
  }

  void FrameLoader::Init() noexcept
  {
    // This somewhat odd set of steps gives the frame an initial empty document.
    auto loader = DocumentLoader::Create(net::ResourceRequest(net::Url(u8"about:blank")), SubstituteData());
    loader->AttachToFrame(_frame);
    _documentLoader = loader;
    _isDisplayingInitialEmptyDocument = true;
    loader->CommitInitialEmptyDocument();
    _state = FrameState::Complete;
  }

  CachePolicy FrameLoader::SubresourceCachePolicy() const noexcept
  {
    if (_isComplete)
    {
      return CachePolicy::Use;
    }
    return _loadType == FrameLoadType::Reload ? CachePolicy::Reload : CachePolicy::Use;
  }

  void FrameLoader::Load(FrameLoadRequest request) noexcept
  {
    auto &resourceRequest = request.GetResourceRequest();

    // A load of a fragment of the document's URL (without data of its own) navigates within the document.
    if (!request.GetSubstituteData() && _frame.Document()
        && ShouldPerformFragmentNavigation(resourceRequest.Url()))
    {
      LoadInSameDocument(resourceRequest.Url());
      return;
    }

    NavigationAction action(resourceRequest, false, nullptr, null, request.SourceElement());
    auto substituteData = request.GetSubstituteData();
    LoadWithNavigationAction(resourceRequest, krys::move(action), FrameLoadType::Standard,
                             krys::move(substituteData));
  }

  void FrameLoader::LoadWithNavigationAction(const net::ResourceRequest &request, NavigationAction &&action,
                                             FrameLoadType loadType,
                                             Maybe<SubstituteData> &&substituteData) noexcept
  {
    // Reloading a document the host gave the markup of loads the markup again.
    if (!substituteData && loadType == FrameLoadType::Reload && _documentLoader
        && _documentLoader->GetSubstituteData() && request.Url() == _documentLoader->DocumentUrl())
    {
      substituteData = _documentLoader->GetSubstituteData();
    }

    auto loader = DocumentLoader::Create(net::ResourceRequest(request), krys::move(substituteData));
    loader->SetTriggeringAction(krys::move(action));

    StopAllLoaders();

    _loadType = loadType;
    loader->AttachToFrame(_frame);
    _provisionalDocumentLoader = loader;
    _state = FrameState::Provisional;

    _client->DispatchDidStartProvisionalLoad();

    // The client may have stopped the load.
    if (_provisionalDocumentLoader != loader.get())
    {
      return;
    }
    loader->StartLoadingMainResource();
  }

  void FrameLoader::StopAllLoaders() noexcept
  {
    if (RefPtr<DocumentLoader> provisional = _provisionalDocumentLoader)
    {
      provisional->StopLoading();
    }
  }

  void FrameLoader::Reload() noexcept
  {
    RefPtr<DocumentLoader> documentLoader = _documentLoader;
    if (!documentLoader || documentLoader->Request().Url().IsEmpty())
    {
      return;
    }

    net::ResourceRequest request(documentLoader->Request());
    NavigationAction action(request, false);
    LoadWithNavigationAction(request, krys::move(action), FrameLoadType::Reload, null);
  }

  void FrameLoader::TransitionToCommittedForNewPage() noexcept
  {
    RefPtr<FrameView> oldView = ShareRefPtr(_frame.View());
    if (!oldView)
    {
      return;
    }
    _frame.CreateView(SizeI(oldView->Width(), oldView->Height()), oldView->FixedLayoutSize(),
                      oldView->UseFixedLayout());
  }

  void FrameLoader::CommitProvisionalLoad() noexcept
  {
    RefPtr<DocumentLoader> loader = _provisionalDocumentLoader;
    if (!loader)
    {
      return;
    }

    // The old document is unloaded, unless it's the document of an SVG image, which fires no events.
    if (_documentLoader && !_frame.GetBooeyClient().IsSVGImageBooeyClient())
    {
      DispatchUnloadEvents();
    }
    // The unload handlers may have started another load.
    if (_provisionalDocumentLoader != loader)
    {
      return;
    }

    if (_documentLoader && _documentLoader != loader)
    {
      _documentLoader->DetachFromFrame();
    }
    _documentLoader = loader;
    _provisionalDocumentLoader = nullptr;

    // Nothing else can interrupt this commit - set the Provisional->Committed transition in stone
    _state = FrameState::CommittedPage;
    _isDisplayingInitialEmptyDocument = false;
    _isComplete = false;
    _didCallImplicitClose = false;
    _wasUnloadEventEmitted = false;
    TransitionToCommittedForNewPage();
  }

  namespace
  {
    /// @brief Counts an unload event's dispatch in the document's unload counter, for as long as it lasts.
    class UnloadCountIncrementer
    {
      KRYS_NON_COPY_MOVABLE(UnloadCountIncrementer);

    private:
      RefPtr<Document> _document;

    public:
      explicit UnloadCountIncrementer(Document *document) noexcept : _document(ShareRefPtr(document))
      {
        if (_document)
        {
          _document->IncrementUnloadCounter();
        }
      }

      ~UnloadCountIncrementer() noexcept
      {
        if (_document)
        {
          _document->DecrementUnloadCounter();
        }
      }
    };
  }

  // https://html.spec.whatwg.org/multipage/document-lifecycle.html#unload-a-document
  void FrameLoader::DispatchUnloadEvents() noexcept
  {
    RefPtr<boo::Document> document = ShareRefPtr(_frame.Document());
    if (!document || _pageDismissalEventBeingDispatched)
    {
      return;
    }

    // 1. Assert: this is running as part of a task queued on oldDocument's relevant agent's event loop.
    // 2.-4. (There's no unload timing info, as there's no navigation timing.)
    // 5. Let intendToKeepInBfcache be true if the user agent intends to keep oldDocument alive in a session
    //    history entry, such that it can later be used for history traversal.
    // (There's no session history: it's false.)
    // 6. Let eventLoop be oldDocument's relevant agent's event loop.
    // 7. Increase eventLoop's termination nesting level by 1.
    // (There's no script to terminate.)
    // 8. Increase oldDocument's unload counter by 1.
    UnloadCountIncrementer unloadCountIncrementer(document.get());

    // (The events are fired once, after the document's load event.)
    if (_didCallImplicitClose && !_wasUnloadEventEmitted)
    {
      SetForScope dismissal(_pageDismissalEventBeingDispatched, true);
      // 9. If intendToKeepInBfcache is false, then set oldDocument's salvageable state to false.
      // 10. If oldDocument's page showing is true:
      // 10a. Set oldDocument's page showing to false.
      // 10b. Fire a page transition event named pagehide at oldDocument's relevant global object with
      //      oldDocument's salvageable state.
      document->DispatchPagehideEvent(false);

      // 10c. Update the visibility state of oldDocument to "hidden".
      document->SetVisibilityHiddenDueToDismissal();

      // 12. If oldDocument's salvageable state is false, then fire an event named unload at oldDocument's
      //     relevant global object, with legacy target override flag set.
      if (RefPtr<Window> window = ShareRefPtr(document->Window()))
      {
        auto unloadEvent = Event::Create(event_names::Unload, false, false);
        (void)dom::EventDispatcher::DispatchToTarget(*unloadEvent, window.get(), true);
      }
      _wasUnloadEventEmitted = true;
    }
  }

  void FrameLoader::DidBeginDocument() noexcept
  {
    // A document written into the frame (as an SVG image's is, after its initial empty one) completes anew.
    _isComplete = false;
    _didCallImplicitClose = false;
    _client->DispatchDidClearWindowObject();
    if (_state == FrameState::CommittedPage)
    {
      _client->DispatchDidCommitLoad();
    }
  }

  void FrameLoader::FinishedLoading(DocumentLoader &loader) noexcept
  {
    if (&loader != _documentLoader.get())
    {
      return;
    }
    CheckCompleted();
  }

  void FrameLoader::LoadDone() noexcept
  {
    CheckCompleted();
  }

  void FrameLoader::CheckCompleted() noexcept
  {
    // Have we completed before?
    if (_isComplete)
    {
      return;
    }

    RefPtr<boo::Document> document = ShareRefPtr(_frame.Document());
    if (!document)
    {
      return;
    }

    // Resource loads are kicked off by style resolution, so it's brought up to date first.
    document->UpdateStyleIfNeeded();

    // Are we still parsing, or waiting for the main resource?
    if (document->Parsing() || (_documentLoader && _documentLoader->IsLoadingMainResource()))
    {
      return;
    }

    // Still waiting for images, style sheets or fonts?
    if (document->GetCachedResourceLoader().RequestCount() || !document->HaveStylesheetsLoaded())
    {
      return;
    }

    // Still waiting for elements that don't go through a frame loader?
    if (document->IsDelayingLoadEvent())
    {
      return;
    }

    // OK, completed.
    _isComplete = true;
    dom::DocumentAlgorithms::UpdateCurrentDocumentReadiness(*document, html::DocumentReadyState::Complete);

    CheckCallImplicitClose();
    CheckLoadComplete();
  }

  void FrameLoader::CheckCallImplicitClose() noexcept
  {
    RefPtr<boo::Document> document = ShareRefPtr(_frame.Document());
    if (_didCallImplicitClose || !document || document->Parsing() || document->IsDelayingLoadEvent())
    {
      return;
    }
    _didCallImplicitClose = true;
    document->ImplicitClose();
  }

  void FrameLoader::CheckLoadComplete() noexcept
  {
    if (_state != FrameState::CommittedPage || !_isComplete)
    {
      return;
    }
    _state = FrameState::Complete;
    _loadType = FrameLoadType::Standard;
    _client->DispatchDidFinishLoad();
  }

  void FrameLoader::ReceivedMainResourceError(DocumentLoader &loader,
                                              const net::ResourceError &error) noexcept
  {
    if (&loader == _provisionalDocumentLoader.get())
    {
      // The frame keeps its document.
      _provisionalDocumentLoader->DetachFromFrame();
      _provisionalDocumentLoader = nullptr;
      _state = FrameState::Complete;
      _client->DispatchDidFailProvisionalLoad(error);
      return;
    }

    if (&loader == _documentLoader.get())
    {
      _state = FrameState::Complete;
      _client->DispatchDidFailLoad(error);
    }
  }

  void FrameLoader::ChangeLocation(const net::Url &url, DOMStringAtom passedTarget, Event *triggeringEvent,
                                   ReferrerPolicy referrerPolicy, DOMStringAtom downloadAttribute,
                                   Element *sourceElement) noexcept
  {
    auto *document = _frame.Document();
    if (!document)
    {
      return;
    }

    FrameLoadRequest frameLoadRequest(ShareRef(*document), net::ResourceRequest(url),
                                      krys::move(passedTarget), krys::move(downloadAttribute));
    frameLoadRequest.SetReferrerPolicy(referrerPolicy);
    frameLoadRequest.SetSourceElement(sourceElement);
    ChangeLocation(krys::move(frameLoadRequest), triggeringEvent);
  }

  void FrameLoader::ChangeLocation(FrameLoadRequest frameRequest, Event *triggeringEvent) noexcept
  {
    krys_assert(frameRequest.GetResourceRequest().HttpMethod() == u8"GET");

    auto *document = _frame.Document();
    // https://html.spec.whatwg.org/multipage/browsing-the-web.html#navigate
    // 9. If navigable's active document's unload counter is greater than 0, then return.
    if (!document || !IsNavigationAllowed())
    {
      return;
    }

    if (frameRequest.FrameName().View().empty())
    {
      frameRequest.SetFrameName(DOMStringAtom(document->BaseTarget()));
    }

    // The target is ignored when the download attribute is set, as the link is downloaded rather than
    // followed.
    bool isDownload = frameRequest.DownloadAttribute() != null;
    auto effectiveFrameName = isDownload ? null : frameRequest.FrameName();
    bool targetsThisFrame = FindFrameForNavigation(effectiveFrameName.View()) == &_frame;

    const auto &request = frameRequest.GetResourceRequest();
    NavigationAction action(request, false, triggeringEvent, frameRequest.DownloadAttribute(),
                            frameRequest.SourceElement());

    // Scroll to the fragment even if the URL is exactly the same, so pages with `#` links work.
    if (targetsThisFrame && !isDownload && ShouldPerformFragmentNavigation(request.Url()))
    {
      LoadInSameDocument(request.Url());
      return;
    }

    RequestNavigation(action, request, nullptr,
                      targetsThisFrame ? DOMStringView() : effectiveFrameName.View());
  }

  void FrameLoader::SubmitForm(Ref<FormSubmission> &&submission) noexcept
  {
    krys_assert(submission->GetMethod() == FormSubmission::Method::Post
                || submission->GetMethod() == FormSubmission::Method::Get);

    auto *document = _frame.Document();
    if (!document || submission->Action().IsEmpty())
    {
      return;
    }

    // A target naming another browsing context is kept, for the host to open; one naming this frame is
    // cleared.
    if (FindFrameForNavigation(submission->Target()))
    {
      submission->ClearTarget();
    }

    if (submission->WasCancelled())
    {
      return;
    }

    // https://html.spec.whatwg.org/multipage/form-control-infrastructure.html#form-submission-algorithm
    // 26a. Let referrerPolicy be the empty string.
    // 26b. If the form element's link types include the noreferrer keyword, then set referrerPolicy to
    //      "no-referrer".
    // (Skipped: there are no referrers, so the policy would change nothing; see DIFFERENCES.md.)

    // https://html.spec.whatwg.org/multipage/form-control-infrastructure.html#plan-to-navigate
    // The navigation is planned for a later task, which a later submission of the form cancels.
    html::EventLoop::Current().QueueTask(
      html::TaskSource::DOMManipulation, *document,
      [this, frame = Ref<boo::Frame>(ShareRef(_frame)), submission = krys::move(submission)]
      {
        auto *document = _frame.Document();
        if (submission->WasCancelled() || !document || !IsNavigationAllowed())
        {
          return;
        }
        auto *form = submission->Form();
        FrameLoadRequest frameLoadRequest(ShareRef(form ? form->NodeDocument() : *document),
                                          net::ResourceRequest(), null);
        submission->PopulateFrameLoadRequest(frameLoadRequest);
        const auto &request = frameLoadRequest.GetResourceRequest();
        NavigationAction action(request, true, submission->Event(), null, form);
        RequestNavigation(action, request, submission.get(), frameLoadRequest.FrameName().View());
      });
  }

  void FrameLoader::RequestNavigation(const NavigationAction &action, const net::ResourceRequest &request,
                                      const FormSubmission *formSubmission, DOMStringView frameName) noexcept
  {
    if (frameName.empty())
    {
      _client->DispatchNavigationRequested(action, request, formSubmission);
      return;
    }

    // https://html.spec.whatwg.org/multipage/document-sequences.html#the-rules-for-choosing-a-navigable
    // 8. If chosen is null, then a new top-level traversable is being requested: ... If the user agent has
    //    been configured such that in this instance it will create a new top-level traversable:
    // 8a. Consume user activation of currentNavigable's active window.
    // (The host opens the new window, if it wants.)
    if (auto *window = _frame.Document() ? _frame.Document()->Window() : nullptr)
    {
      (void)window->ConsumeTransientActivation();
    }
    _client->DispatchNewWindowRequested(action, request, formSubmission, frameName);
  }

  boo::Frame *FrameLoader::FindFrameForNavigation(DOMStringView name) const noexcept
  {
    // https://html.spec.whatwg.org/multipage/document-sequences.html#the-rules-for-choosing-a-navigable
    // 1. Let chosen be null.
    // 2. Let windowType be "existing or none".
    // 3. Let sandboxingFlagSet be currentNavigable's active document's active sandboxing flag set.
    // 4. If name is the empty string or an ASCII case-insensitive match for "_self", then set chosen to
    //    currentNavigable.
    // 5. Otherwise, if name is an ASCII case-insensitive match for "_parent", set chosen to
    //    currentNavigable's parent, if any, and currentNavigable otherwise.
    // 6. Otherwise, if name is an ASCII case-insensitive match for "_top", set chosen to currentNavigable's
    //    traversable navigable.
    // (The frame has no parent, and is its traversable navigable.)
    if (name.empty() || text::IsASCIICaselessEqual(name, u8"_self")
        || text::IsASCIICaselessEqual(name, u8"_parent") || text::IsASCIICaselessEqual(name, u8"_top"))
    {
      return &_frame;
    }
    // 7. Otherwise, if name is not an ASCII case-insensitive match for "_blank", then set chosen to the
    //    result of finding a navigable by target name given name and currentNavigable.
    // (The frame has no name, as there's no window.name, and there are no other browsing contexts.)
    // 8. If chosen is null, then a new top-level traversable is being requested.
    // (RequestNavigation asks the host for one.)
    return nullptr;
  }

  bool FrameLoader::ShouldReload(const net::Url &currentUrl, const net::Url &destinationUrl) noexcept
  {
    // This function implements the rule: "Don't reload if navigating by fragment within the same URL, but do
    // reload if going to a new URL or to the same URL with no fragment identifier at all."
    if (!destinationUrl.HasFragmentIdentifier())
    {
      return true;
    }
    return !net::EqualIgnoringFragmentIdentifier(currentUrl, destinationUrl);
  }

  bool FrameLoader::ShouldPerformFragmentNavigation(const net::Url &url) const noexcept
  {
    return !ShouldReload(_frame.Document()->GetUrl(), url);
  }

  void FrameLoader::LoadInSameDocument(const net::Url &url) noexcept
  {
    Ref<boo::Frame> protectedFrame = ShareRef(_frame);
    RefPtr<boo::Document> document = ShareRefPtr(_frame.Document());
    if (!document)
    {
      return;
    }

    auto oldUrl = document->GetUrl();
    document->SetUrl(url);

    // If we were in the autoscroll mode we want to stop it before following the link to the anchor.
    bool hashChange = net::EqualIgnoringFragmentIdentifier(url, oldUrl)
                      && (url.FragmentIdentifier() != oldUrl.FragmentIdentifier()
                          || url.HasFragmentIdentifier() != oldUrl.HasFragmentIdentifier());
    if (hashChange)
    {
      _frame.GetEventHandler().StopAutoscrollTimer();
    }

    // We need to scroll to the fragment whether or not a hash change occurred, since the user might have
    // scrolled since the previous navigation.
    ScrollToFragment(url);

    _client->DispatchDidNavigateWithinPage();
  }

  void FrameLoader::FinishedParsing() noexcept
  {
    Ref<boo::Frame> frame = ShareRef(_frame);
    if (_state == FrameState::CommittedPage)
    {
      _client->DispatchDidFinishDocumentLoad();
    }

    CheckCompleted();

    // Check if the scrollbars are really needed for the content. If not, remove them, relayout, and repaint.
    if (auto *view = frame->View())
    {
      view->RestoreScrollbar();
    }

    if (auto *document = frame->Document())
    {
      ScrollToFragment(document->GetUrl());
    }
  }

  void FrameLoader::ScrollToFragment(const net::Url &url) noexcept
  {
    RefPtr<FrameView> view = ShareRefPtr(_frame.View());
    RefPtr<boo::Document> document = ShareRefPtr(_frame.Document());
    if (!view || !document)
    {
      return;
    }

    // https://html.spec.whatwg.org/multipage/browsing-the-web.html#try-to-scroll-to-the-fragment
    if (!document->HaveStylesheetsLoaded())
    {
      document->SetGotoAnchorNeededAfterStylesheetsLoad(true);
    }
    else
    {
      (void)view->ScrollToFragment(url);
    }
  }
}
