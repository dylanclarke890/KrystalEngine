#pragma once

#include "Krystal.Booey/DOM/Types/DOMString.hpp"
#include "Krystal.Booey/Loader/Cache/CachePolicy.hpp"
#include "Krystal.Booey/Loader/FetchOptions.hpp"
#include "Krystal.Booey/Loader/FrameLoaderTypes.hpp"
#include "Krystal.Booey/Loader/LocalFrameLoaderClient.hpp"
#include "Krystal.Booey/Loader/SubstituteData.hpp"
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
  class DocumentLoader;
  class FormSubmission;
  class FrameLoadRequest;
  class LocalFrame;
  class NavigationAction;
  class ResourceError;
  class ResourceRequest;

  /// @brief Where a frame is in loading a document.
  enum class FrameState : uint8
  {
    /// @brief A load started; the frame still shows its old document.
    Provisional,
    /// @brief The new document is the frame's, and loading.
    CommittedPage,
    /// @brief Nothing is loading.
    Complete,
  };

  /// @brief Loads documents into a frame as the host asks, provisionally until their data arrives and they
  /// commit. Booey doesn't navigate by itself: links and form submissions are handed to the host, which loads
  /// what it wants, and only a link to a fragment of the document navigates, within it.
  class FrameLoader
  {
    KRYS_NON_COPY_MOVABLE(FrameLoader);

  private:
    LocalFrame &_frame;
    UniquePtr<LocalFrameLoaderClient> _client;
    FrameLoadType _loadType {FrameLoadType::Standard};
    /// @brief The loader of the frame's document.
    RefPtr<DocumentLoader> _documentLoader;
    /// @brief The loader of the document the frame is loading, until it commits.
    RefPtr<DocumentLoader> _provisionalDocumentLoader;
    FrameState _state {FrameState::Complete};
    /// @brief Whether the frame shows the initial empty document (`about:blank`) it was given.
    bool _isDisplayingInitialEmptyDocument {false};
    /// @brief Whether the document finished loading (its load event fired), or there's no load.
    bool _isComplete {true};
    /// @brief Whether the document's load event was fired (implicitClose).
    bool _didCallImplicitClose {true};
    /// @brief Whether the document's unload events were fired.
    bool _wasUnloadEventEmitted {false};
    /// @brief Whether the document's pagehide or unload events are being dispatched.
    bool _pageDismissalEventBeingDispatched {false};

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

    KRYS_NODISCARD FrameState State() const noexcept
    {
      return _state;
    }

    /// @brief The loader of the frame's document, if it has one.
    KRYS_NODISCARD DocumentLoader *GetDocumentLoader() const noexcept;

    /// @brief The loader of the document the frame is loading, if any.
    KRYS_NODISCARD DocumentLoader *ProvisionalDocumentLoader() const noexcept;

    /// @brief Whether a document is loading in the frame: provisionally, or its data still to come.
    KRYS_NODISCARD bool IsLoading() const noexcept;

    KRYS_NODISCARD bool IsDisplayingInitialEmptyDocument() const noexcept
    {
      return _isDisplayingInitialEmptyDocument;
    }

    /// @brief How the document's subresources use the memory cache, from the load type while the document
    /// loads: revalidated for a reload.
    KRYS_NODISCARD CachePolicy SubresourceCachePolicy() const noexcept;

    /// @brief Gives the frame its initial empty document (`about:blank`), at once.
    /// @see https://html.spec.whatwg.org/#initialise-the-document-object
    void Init() noexcept;

    /// @brief Loads @p request into the frame, as the host asks: its URL, or the substitute data it carries.
    /// A fragment of the document's URL (without data of its own) navigates within the document; another
    /// document commits on a later task.
    void Load(FrameLoadRequest &&request) noexcept;

    /// @brief Stops the frame's provisional load, if any; the client is told it failed (cancelled).
    void StopAllLoaders() noexcept;

    /// @brief Loads the document again, as the host asks; its form state isn't restored.
    void Reload() noexcept;

    /// @brief Whether the frame may navigate: not while its document's `pagehide` or `unload` is being
    /// dispatched.
    /// @see https://html.spec.whatwg.org/#navigate (step 9)
    KRYS_NODISCARD bool IsNavigationAllowed() const noexcept
    {
      return !_pageDismissalEventBeingDispatched;
    }

    /// @brief Follows a link to @p url: a fragment of the document's URL in the frame's own browsing context
    /// navigates within the document; anything else is handed to the host.
    /// @param passedTarget The browsing context to navigate (`_self` and the others, or a name); empty for
    /// the document's base target.
    /// @param triggeringEvent The event that started the navigation (a click), if any.
    /// @param downloadAttribute The link's `download` attribute; null if it has none.
    /// @param sourceElement The link, if any.
    void ChangeLocation(const url::URL &url, dom::DOMStringAtom passedTarget, dom::Event *triggeringEvent,
                        ReferrerPolicy referrerPolicy, dom::DOMStringAtom downloadAttribute,
                        dom::Element *sourceElement = nullptr) noexcept;

    /// @brief Navigates as @p frameRequest asks, to a `GET` request.
    void ChangeLocation(FrameLoadRequest &&frameRequest, dom::Event *triggeringEvent = nullptr) noexcept;

    /// @brief Hands a form submission (a `GET` or a `POST`) to the host.
    void SubmitForm(Ref<FormSubmission> &&submission) noexcept;

    /// @brief The frame a navigation targeting @p name navigates: this frame for an empty name, `_self`,
    /// `_parent` and `_top` (a frame is the only one in its tree); null for `_blank` and other names, which
    /// would open a new browsing context.
    KRYS_NODISCARD LocalFrame *FindFrameForNavigation(dom::DOMStringView name) const noexcept;

    /// @brief Run when the parser stops parsing the frame's document: scrolls to its URL's fragment.
    void FinishedParsing() noexcept;

    /// @brief The provisional load's data arrived: it becomes the frame's document loader.
    void CommitProvisionalLoad() noexcept;

    /// @brief The committed document was made and is the frame's: the client sets it up (custom elements)
    /// before it's parsed, and hears that the load committed.
    void DidBeginDocument() noexcept;

    /// @brief @p loader's document has all its data.
    void FinishedLoading(DocumentLoader &loader) noexcept;

    /// @brief A load of the document (a subresource) is done: checks whether the document has loaded.
    void LoadDone() noexcept;

    /// @brief Whether the document has loaded (parsed, its subresources and anything else delaying its load
    /// event done), and if so, fires its load event and tells the client.
    /// @see https://html.spec.whatwg.org/#the-end
    void CheckCompleted() noexcept;

    KRYS_NODISCARD bool IsComplete() const noexcept
    {
      return _isComplete;
    }

    /// @brief @p loader's main resource couldn't be loaded (or the load was stopped).
    void ReceivedMainResourceError(DocumentLoader &loader, const ResourceError &error) noexcept;

    /// @brief Whether navigating from @p currentURL to @p destinationURL loads a document, rather than
    /// navigating to a fragment: unless @p destinationURL has a fragment and is otherwise @p currentURL.
    KRYS_NODISCARD static bool ShouldReload(const url::URL &currentURL,
                                            const url::URL &destinationURL) noexcept;

  private:
    /// @brief Loads @p request (or @p substituteData) as a new document. A reload of a document the host gave
    /// the markup of loads that markup again.
    void LoadWithNavigationAction(const ResourceRequest &request, NavigationAction &&action,
                                  FrameLoadType loadType, Maybe<SubstituteData> &&substituteData) noexcept;

    /// @brief Hands the navigation @p action started to the host, for the browsing context @p frameName
    /// names (a new one for `_blank` or a name no frame has).
    void RequestNavigation(const NavigationAction &action, const ResourceRequest &request,
                           const FormSubmission *formSubmission, dom::DOMStringView frameName) noexcept;

    /// @brief The committed document gets a view of its own, as a new page's would: scrolled to its top, not
    /// yet laid out.
    void TransitionToCommittedForNewPage() noexcept;

    /// @brief Whether the load is complete, and if so, tells the client.
    void CheckLoadComplete() noexcept;

    /// @brief Fires the document's load event, once it's parsed and nothing delays it.
    void CheckCallImplicitClose() noexcept;

    /// @brief Fires `pagehide` and `unload` at the document's window, as it's left.
    /// @see https://html.spec.whatwg.org/#unload-a-document
    void DispatchUnloadEvents() noexcept;

    /// @brief Whether a navigation to @p url navigates within the document: to a fragment of its URL.
    KRYS_NODISCARD bool ShouldPerformFragmentNavigation(const url::URL &url) const noexcept;

    /// @brief Navigates to @p url, which differs from the document's URL at most in its fragment: the
    /// document takes the URL and scrolls to its fragment.
    void LoadInSameDocument(const url::URL &url) noexcept;

    /// @brief Scrolls to @p url's fragment, once the document's style sheets have loaded.
    /// @see https://html.spec.whatwg.org/#try-to-scroll-to-the-fragment
    void ScrollToFragment(const url::URL &url) noexcept;
  };
}
