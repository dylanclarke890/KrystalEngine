#include "Krystal.Booey/Loader/FrameLoader.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/HTML/HTMLFormControlElement.hpp"
#include "Krystal.Booey/HTML/HTMLFormElement.hpp"
#include "Krystal.Booey/Loader/FormSubmission.hpp"
#include "Krystal.Booey/Loader/FrameLoadRequest.hpp"
#include "Krystal.Booey/Loader/NavigationAction.hpp"
#include "Krystal.Booey/Page/LocalFrame.hpp"
#include "Krystal.Booey/Page/LocalFrameView.hpp"
#include "Krystal.Core/Text/ASCII.hpp"

namespace krys::boo
{
  FrameLoader::FrameLoader(LocalFrame &frame, UniquePtr<LocalFrameLoaderClient> &&client) noexcept
      : _frame(frame), _client(client ? krys::move(client) : CreateUnique<EmptyFrameLoaderClient>())
  {
  }

  FrameLoader::~FrameLoader() noexcept = default;

  void FrameLoader::ChangeLocation(const url::URL &url, dom::DOMStringAtom passedTarget,
                                   dom::Event *triggeringEvent, ReferrerPolicy referrerPolicy,
                                   Maybe<NewFrameOpenerPolicy> openerPolicy,
                                   dom::DOMStringAtom downloadAttribute,
                                   NavigationHistoryBehavior historyBehavior,
                                   dom::Element *sourceElement) noexcept
  {
    auto *document = _frame.Document();
    if (!document)
    {
      return;
    }

    FrameLoadRequest frameLoadRequest(ShareRef(*document), ResourceRequest(url), passedTarget,
                                      downloadAttribute);
    frameLoadRequest.SetNewFrameOpenerPolicy(
      openerPolicy.value_or(referrerPolicy == ReferrerPolicy::NoReferrer ? NewFrameOpenerPolicy::Suppress
                                                                         : NewFrameOpenerPolicy::Allow));
    frameLoadRequest.SetReferrerPolicy(referrerPolicy);
    frameLoadRequest.SetNavigationHistoryBehavior(historyBehavior);
    frameLoadRequest.SetSourceElement(sourceElement);
    ChangeLocation(krys::move(frameLoadRequest), triggeringEvent);
  }

  void FrameLoader::ChangeLocation(FrameLoadRequest &&frameRequest, dom::Event *triggeringEvent) noexcept
  {
    krys_assert(frameRequest.GetResourceRequest().HTTPMethod() == u8"GET");

    if (frameRequest.FrameName().View().empty())
    {
      if (auto *document = _frame.Document())
      {
        frameRequest.SetFrameName(dom::DOMStringAtom(document->BaseTarget()));
      }
    }

    LoadFrameRequest(krys::move(frameRequest), triggeringEvent, nullptr);
  }

  void FrameLoader::SubmitForm(Ref<FormSubmission> &&submission) noexcept
  {
    krys_assert(submission->GetMethod() == FormSubmission::Method::Post
                || submission->GetMethod() == FormSubmission::Method::Get);

    if (!_frame.GetPage())
    {
      return;
    }

    if (submission->Action().IsEmpty())
    {
      return;
    }

    auto *form = submission->Form();
    auto &requestingDocument = form ? form->NodeDocument() : *_frame.Document();

    // A target naming another browsing context is kept, for the client's new window policy; one naming this
    // frame is cleared.
    if (FindFrameForNavigation(submission->Target()))
    {
      submission->ClearTarget();
    }

    if (submission->WasCancelled())
    {
      return;
    }

    FrameLoadRequest frameLoadRequest(ShareRef(requestingDocument), ResourceRequest(),
                                      dom::DOMStringAtom::Null());
    submission->PopulateFrameLoadRequest(frameLoadRequest);
    frameLoadRequest.SetNavigationHistoryBehavior(NavigationHistoryBehavior::Push);
    frameLoadRequest.SetSourceElement(form);
    RefPtr<dom::Event> event = ShareRefPtr(submission->Event());
    LoadFrameRequest(krys::move(frameLoadRequest), event.get(), krys::move(submission));
  }

  void FrameLoader::LoadFrameRequest(FrameLoadRequest &&request, dom::Event *event,
                                     RefPtr<FormSubmission> &&formSubmission) noexcept
  {
    krys_assert(_frame.Document());

    // FIXME: Reloads, and history locking, once there's history.
    auto loadType = FrameLoadType::Standard;

    if (request.GetResourceRequest().HTTPMethod() == u8"POST")
    {
      LoadPostRequest(krys::move(request), loadType, event, krys::move(formSubmission));
    }
    else
    {
      LoadURL(krys::move(request), loadType, event, krys::move(formSubmission));
    }
  }

  void FrameLoader::LoadURL(FrameLoadRequest &&frameLoadRequest, FrameLoadType newLoadType, dom::Event *event,
                            RefPtr<FormSubmission> &&formSubmission) noexcept
  {
    krys_assert(frameLoadRequest.GetResourceRequest().HTTPMethod() == u8"GET");

    // The target is ignored when the download attribute is set, as the link is downloaded rather than
    // followed.
    auto effectiveFrameName = frameLoadRequest.DownloadAttribute() == dom::DOMStringAtom::Null()
                                ? frameLoadRequest.FrameName()
                                : dom::DOMStringAtom::Null();
    bool isFormSubmission = formSubmission != nullptr;

    auto *targetFrame = isFormSubmission ? nullptr : FindFrameForNavigation(effectiveFrameName.View());
    if (targetFrame && targetFrame != &_frame)
    {
      frameLoadRequest.SetFrameName(dom::DOMStringAtom(u8"_self"));
      targetFrame->Loader().LoadURL(krys::move(frameLoadRequest), newLoadType, event,
                                    krys::move(formSubmission));
      return;
    }

    const auto &newURL = frameLoadRequest.GetResourceRequest().URL();
    ResourceRequest request(newURL);

    NavigationAction action(request, newLoadType, isFormSubmission, event,
                            frameLoadRequest.DownloadAttribute(), frameLoadRequest.SourceElement());
    action.SetIsRequestFromClientOrUserInput(frameLoadRequest.IsRequestFromClientOrUserInput());
    action.SetNewFrameOpenerPolicy(frameLoadRequest.GetNewFrameOpenerPolicy());

    auto historyHandling = frameLoadRequest.GetNavigationHistoryBehavior();
    auto &document = *_frame.Document();
    if (!IsReload(newLoadType) && historyHandling == NavigationHistoryBehavior::Auto)
    {
      historyHandling =
        document.GetURL() == newURL ? NavigationHistoryBehavior::Replace : NavigationHistoryBehavior::Push;
    }
    frameLoadRequest.SetNavigationHistoryBehavior(historyHandling);

    if (!targetFrame && !effectiveFrameName.View().empty())
    {
      CheckNewWindowPolicy(action, request, formSubmission.get(), effectiveFrameName.View());
      return;
    }

    // Scroll to the fragment even if the URL is exactly the same, so pages with `#` links work.
    if (ShouldPerformFragmentNavigation(isFormSubmission, request.HTTPMethod(), newLoadType, newURL))
    {
      auto policy = _client->DispatchDecidePolicyForNavigationAction(action, request, formSubmission.get());
      ContinueFragmentScrollAfterNavigationPolicy(request, policy == PolicyAction::Use, historyHandling);
      return;
    }

    auto policy = _client->DispatchDecidePolicyForNavigationAction(action, request, formSubmission.get());
    if (policy != PolicyAction::Use)
    {
      // FIXME: Downloads, once there are any.
      return;
    }

    // FIXME: Loading the new document (loadWithNavigationAction), once there's document loading (B3).
  }

  void FrameLoader::LoadPostRequest(FrameLoadRequest &&request, FrameLoadType loadType, dom::Event *event,
                                    RefPtr<FormSubmission> &&formSubmission) noexcept
  {
    auto frameName = request.FrameName();

    ResourceRequest workingResourceRequest(request.GetResourceRequest().URL());
    workingResourceRequest.SetHTTPMethod(u8"POST");

    auto *targetFrame =
      formSubmission || frameName.View().empty() ? nullptr : FindFrameForNavigation(frameName.View());

    NavigationAction action(workingResourceRequest, loadType, true, event);
    action.SetIsRequestFromClientOrUserInput(request.IsRequestFromClientOrUserInput());
    action.SetNewFrameOpenerPolicy(request.GetNewFrameOpenerPolicy());
    action.SetSourceElement(request.SourceElement());

    if (!frameName.View().empty() && !targetFrame)
    {
      CheckNewWindowPolicy(action, workingResourceRequest, formSubmission.get(), frameName.View());
      return;
    }

    auto policy =
      _client->DispatchDecidePolicyForNavigationAction(action, workingResourceRequest, formSubmission.get());
    if (policy != PolicyAction::Use)
    {
      return;
    }

    // FIXME: Loading the response as the new document (loadWithNavigationAction), once there's document
    // loading (B3).
  }

  void FrameLoader::CheckNewWindowPolicy(const NavigationAction &action, const ResourceRequest &request,
                                         const FormSubmission *formSubmission,
                                         dom::DOMStringView frameName) noexcept
  {
    auto policy = _client->DispatchDecidePolicyForNewWindowAction(action, request, formSubmission, frameName);
    // FIXME: Opening the new browsing context (continueLoadAfterNewWindowPolicy): there are no windows, so
    // the host opens one itself and answers `Ignore`.
    (void)policy;
  }

  LocalFrame *FrameLoader::FindFrameForNavigation(dom::DOMStringView name) const noexcept
  {
    // https://html.spec.whatwg.org/#the-rules-for-choosing-a-navigable
    if (name.empty() || text::IsASCIICaselessEqual(name, utf8_stringview(u8"_self"))
        || text::IsASCIICaselessEqual(name, utf8_stringview(u8"_parent"))
        || text::IsASCIICaselessEqual(name, utf8_stringview(u8"_top")))
    {
      return &_frame;
    }
    // `_blank`, and names no frame has: there's no frame tree.
    return nullptr;
  }

  bool FrameLoader::ShouldReload(const url::URL &currentURL, const url::URL &destinationURL) noexcept
  {
    // This function implements the rule: "Don't reload if navigating by fragment within the same URL, but do
    // reload if going to a new URL or to the same URL with no fragment identifier at all."
    if (!destinationURL.HasFragmentIdentifier())
    {
      return true;
    }
    return !url::EqualIgnoringFragmentIdentifier(currentURL, destinationURL);
  }

  bool FrameLoader::ShouldPerformFragmentNavigation(bool isFormSubmission, utf8_stringview httpMethod,
                                                    FrameLoadType loadType,
                                                    const url::URL &url) const noexcept
  {
    // We don't do this if we are submitting a form with method other than "GET", explicitly reloading, or if
    // the URL does not have a fragment.
    // FIXME: The back and forward cache, framesets and the initial empty document, once there are any.
    return (!isFormSubmission || text::IsASCIICaselessEqual(httpMethod, utf8_stringview(u8"get")))
           && !IsReload(loadType) && loadType != FrameLoadType::Same
           && !ShouldReload(_frame.Document()->GetURL(), url);
  }

  void FrameLoader::ContinueFragmentScrollAfterNavigationPolicy(
    const ResourceRequest &request, bool shouldContinue, NavigationHistoryBehavior historyHandling) noexcept
  {
    if (!shouldContinue)
    {
      return;
    }

    // FIXME: Canceling a provisional load of another document, once there are any.
    bool isRedirect = _loadType == FrameLoadType::RedirectWithLockedBackForwardList;
    LoadInSameDocument(request.URL(), !isRedirect, historyHandling);
  }

  void FrameLoader::LoadInSameDocument(const url::URL &url, bool isNewNavigation,
                                       NavigationHistoryBehavior historyHandling) noexcept
  {
    Ref<LocalFrame> protectedFrame = ShareRef(_frame);
    RefPtr<dom::Document> document = ShareRefPtr(_frame.Document());
    if (!document)
    {
      return;
    }

    auto oldURL = document->GetURL();
    document->SetURL(url::URL(url));

    // FIXME: The session history entry (pushed, or replaced for historyHandling == Replace), once there's
    // history (B3).
    (void)historyHandling;

    bool hashChange = url::EqualIgnoringFragmentIdentifier(url, oldURL)
                      && (url.FragmentIdentifier() != oldURL.FragmentIdentifier()
                          || url.HasFragmentIdentifier() != oldURL.HasFragmentIdentifier());

    // If we were in the autoscroll mode we want to stop it before following the link to the anchor.
    if (hashChange)
    {
      _frame.GetEventHandler().StopAutoscrollTimer();
    }

    // We need to scroll to the fragment whether or not a hash change occurred, since the user might have
    // scrolled since the previous navigation.
    ScrollToFragmentWithParentBoundary(url, isNewNavigation);

    _client->DispatchDidNavigateWithinPage();

    if (hashChange)
    {
      document->EnqueueHashchangeEvent(dom::USVString(oldURL.String()), dom::USVString(url.String()));
      _client->DispatchDidChangeLocationWithinPage();
    }
  }

  void FrameLoader::FinishedParsing() noexcept
  {
    // FIXME: The rest (user scripts, the client's didFinishDocumentLoad, checkCompleted and the scrollbars),
    // with document loading (B3).
    Ref<LocalFrame> frame = ShareRef(_frame);
    if (auto *document = frame->Document())
    {
      ScrollToFragmentWithParentBoundary(document->GetURL());
    }
  }

  void FrameLoader::ScrollToFragmentWithParentBoundary(const url::URL &url, bool isNewNavigation) noexcept
  {
    RefPtr<LocalFrameView> view = ShareRefPtr(_frame.View());
    RefPtr<dom::Document> document = ShareRefPtr(_frame.Document());
    if (!view || !document)
    {
      return;
    }

    // FIXME: Not for a reload or a back and forward load whose history item doesn't restore its scroll
    // position (isSameDocumentReload, itemAllowsScrollRestoration), once there's history.
    (void)isNewNavigation;

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
