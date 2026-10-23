#pragma once

#include "Krystal.Booey/DOM/Types/DOMString.hpp"
#include "Krystal.Booey/Loader/FetchOptions.hpp"
#include "Krystal.Booey/Loader/FrameLoaderTypes.hpp"
#include "Krystal.Booey/Loader/LocalFrameLoaderClient.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"

namespace krys::boo::dom
{
  class Element;
  class Event;
}

namespace krys::boo::url
{
  class URL;
}

namespace krys::boo
{
  class FormSubmission;
  class FrameLoadRequest;
  class LocalFrame;
  class ResourceRequest;

  /// @brief Navigates a frame: links and form submissions ask the frame's loader, which asks its client
  /// whether to navigate and does navigations within the document itself.
  /// @note FIXME: Only navigation within a document. Loading another document (document loaders, the load
  /// lifecycle, the session history, reloads, redirects) comes with document loading (B3), and new windows
  /// and other frames with a frame tree (not planned).
  class FrameLoader
  {
    KRYS_NON_COPY_MOVABLE(FrameLoader);

  private:
    LocalFrame &_frame;
    UniquePtr<LocalFrameLoaderClient> _client;
    FrameLoadType _loadType {FrameLoadType::Standard};

  public:
    /// @param client The host's side of the loader; without one, the loader has an empty client, which
    /// ignores every navigation.
    FrameLoader(LocalFrame &frame, UniquePtr<LocalFrameLoaderClient> &&client) noexcept;
    ~FrameLoader() noexcept;

    KRYS_NODISCARD LocalFrameLoaderClient &Client() const noexcept
    {
      return *_client;
    }

    KRYS_NODISCARD LocalFrame &Frame() const noexcept
    {
      return _frame;
    }

    /// @brief The type of the frame's current load.
    KRYS_NODISCARD FrameLoadType LoadType() const noexcept
    {
      return _loadType;
    }

    /// @brief Navigates to @p url, as following a link does.
    /// @param passedTarget The browsing context to navigate (`_self` and the others, or a name); empty for
    /// the document's base target.
    /// @param triggeringEvent The event that started the navigation (a click), if any.
    /// @param openerPolicy Whether a new browsing context gets an opener; by default, unless the referrer
    /// policy is `no-referrer`.
    /// @param downloadAttribute The link's `download` attribute; null if it has none.
    /// @param sourceElement The link, if any.
    void ChangeLocation(const url::URL &url, dom::DOMStringAtom passedTarget, dom::Event *triggeringEvent,
                        ReferrerPolicy referrerPolicy, Maybe<NewFrameOpenerPolicy> openerPolicy,
                        dom::DOMStringAtom downloadAttribute,
                        NavigationHistoryBehavior historyBehavior = NavigationHistoryBehavior::Auto,
                        dom::Element *sourceElement = nullptr) noexcept;

    /// @brief Navigates as @p frameRequest asks, to a `GET` request.
    void ChangeLocation(FrameLoadRequest &&frameRequest, dom::Event *triggeringEvent = nullptr) noexcept;

    /// @brief Navigates to a form submission (a `GET` or a `POST`).
    // FIXME: Submissions should be scheduled (the navigation scheduler), once there's document loading.
    void SubmitForm(Ref<FormSubmission> &&submission) noexcept;

    /// @brief Loads @p request into the browsing context it names, once the client allows it.
    /// @param formSubmission The form submitted, for a form submission.
    void LoadFrameRequest(FrameLoadRequest &&request, dom::Event *event,
                          RefPtr<FormSubmission> &&formSubmission) noexcept;

    /// @brief The frame a navigation targeting @p name navigates: this frame for an empty name, `_self`,
    /// `_parent` and `_top` (a frame is the only one in its tree); null for `_blank` and other names, which
    /// would open a new browsing context.
    KRYS_NODISCARD LocalFrame *FindFrameForNavigation(dom::DOMStringView name) const noexcept;

    /// @brief Run when the parser stops parsing the frame's document: scrolls to its URL's fragment.
    void FinishedParsing() noexcept;

    /// @brief Whether navigating from @p currentURL to @p destinationURL loads a document, rather than
    /// navigating to a fragment: unless @p destinationURL has a fragment and is otherwise @p currentURL.
    KRYS_NODISCARD static bool ShouldReload(const url::URL &currentURL,
                                            const url::URL &destinationURL) noexcept;

  private:
    void LoadURL(FrameLoadRequest &&frameLoadRequest, FrameLoadType newLoadType, dom::Event *event,
                 RefPtr<FormSubmission> &&formSubmission) noexcept;

    void LoadPostRequest(FrameLoadRequest &&request, FrameLoadType loadType, dom::Event *event,
                         RefPtr<FormSubmission> &&formSubmission) noexcept;

    /// @brief Asks the client to open a new browsing context for @p action.
    void CheckNewWindowPolicy(const NavigationAction &action, const ResourceRequest &request,
                              const FormSubmission *formSubmission, dom::DOMStringView frameName) noexcept;

    KRYS_NODISCARD bool ShouldPerformFragmentNavigation(bool isFormSubmission, utf8_stringview httpMethod,
                                                        FrameLoadType loadType,
                                                        const url::URL &url) const noexcept;

    void ContinueFragmentScrollAfterNavigationPolicy(const ResourceRequest &request, bool shouldContinue,
                                                     NavigationHistoryBehavior historyHandling) noexcept;

    /// @brief Navigates to @p url, which differs from the document's URL at most in its fragment.
    void LoadInSameDocument(const url::URL &url, bool isNewNavigation,
                            NavigationHistoryBehavior historyHandling) noexcept;

    /// @brief Scrolls to @p url's fragment, once the document's style sheets have loaded.
    /// @see https://html.spec.whatwg.org/#try-to-scroll-to-the-fragment
    void ScrollToFragmentWithParentBoundary(const url::URL &url, bool isNewNavigation = true) noexcept;
  };
}
