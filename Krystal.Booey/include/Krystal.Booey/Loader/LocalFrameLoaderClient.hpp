#pragma once

#include "Krystal.Booey/DOM/Types/DOMString.hpp"
#include "Krystal.Booey/Loader/FrameLoaderTypes.hpp"
#include "Krystal.Booey/URL/URL.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"

namespace krys::boo
{
  class FormSubmission;
  class HistoryItem;
  class NavigationAction;
  class ResourceError;
  class ResourceRequest;

  /// @brief What a frame's loader asks of its host: whether to navigate, and what happened.
  /// @note The host answers each navigation as it's asked, rather than later.
  // FIXME: Global history, downloads, the progress and the rest of the client, once there are those.
  class LocalFrameLoaderClient
  {
    KRYS_NON_COPY_MOVABLE(LocalFrameLoaderClient);

  public:
    LocalFrameLoaderClient() noexcept = default;
    virtual ~LocalFrameLoaderClient() noexcept = default;

    /// @brief Decides whether the frame navigates to @p request, which @p action started: `Use` navigates,
    /// `Download` downloads it, and `Ignore` leaves the page as it is (the host may handle it itself).
    /// @param formSubmission The form submitted, for a form submission.
    KRYS_NODISCARD virtual PolicyAction
      DispatchDecidePolicyForNavigationAction(const NavigationAction &action, const ResourceRequest &request,
                                              const FormSubmission *formSubmission) noexcept = 0;

    /// @brief Decides whether a new browsing context named @p frameName (`_blank` or a name no frame has) is
    /// opened for @p request.
    KRYS_NODISCARD virtual PolicyAction
      DispatchDecidePolicyForNewWindowAction(const NavigationAction &action, const ResourceRequest &request,
                                             const FormSubmission *formSubmission,
                                             dom::DOMStringView frameName) noexcept = 0;

    /// @brief Decides whether the frame goes to @p item (back or forward); by default, it does.
    KRYS_NODISCARD virtual boo::ShouldGoToHistoryItem
      ShouldGoToHistoryItem(const HistoryItem &item, IsSameDocumentNavigation isSameDocument) noexcept
    {
      (void)item;
      (void)isSameDocument;
      return boo::ShouldGoToHistoryItem::Yes;
    }

    /// @brief Whether the frame's page may enter the back/forward cache as it's left; by default, it may.
    KRYS_NODISCARD virtual bool CanCachePage() const noexcept
    {
      return true;
    }

    /// @brief The user agent string for @p url, as `navigator.userAgent` reports it.
    KRYS_NODISCARD virtual dom::DOMString UserAgent(const url::URL &url) const noexcept
    {
      (void)url;
      return dom::DOMString(DefaultUserAgent);
    }

    /// @brief Booey's user agent string, unless the host gives its own.
    static constexpr utf8_stringview DefaultUserAgent =
      u8"Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/605.1.15 (KHTML, like Gecko) Booey";

    /// @brief A document started loading in the frame (provisionally: the frame shows its old document).
    virtual void DispatchDidStartProvisionalLoad() noexcept
    {
    }

    /// @brief The provisional load failed (or was stopped): the frame keeps its document.
    virtual void DispatchDidFailProvisionalLoad(const ResourceError &error) noexcept
    {
      (void)error;
    }

    /// @brief The loading document is the frame's now, made and not yet parsed; for a page from the
    /// back/forward cache, the frame still shows the old document until `DidRestoreFromBackForwardCache`.
    virtual void DispatchDidCommitLoad() noexcept
    {
    }

    /// @brief A page from the back/forward cache is the frame's again, its document resumed.
    virtual void DidRestoreFromBackForwardCache() noexcept
    {
    }

    /// @brief A new document (and window) is the frame's, before it's parsed: the host sets it up (its
    /// custom element definitions, its focus state).
    virtual void DispatchDidClearWindowObject() noexcept
    {
    }

    /// @brief The frame's document was parsed.
    virtual void DispatchDidFinishDocumentLoad() noexcept
    {
    }

    /// @brief The frame's document finished loading.
    virtual void DispatchDidFinishLoad() noexcept
    {
    }

    /// @brief The frame's committed document failed to load.
    virtual void DispatchDidFailLoad(const ResourceError &error) noexcept
    {
      (void)error;
    }

    /// @brief The frame navigated within its document (to a fragment).
    virtual void DispatchDidNavigateWithinPage() noexcept
    {
    }

    /// @brief The frame's URL changed within its document: its fragment changed.
    virtual void DispatchDidChangeLocationWithinPage() noexcept
    {
    }

    /// @brief The document added a history entry (`pushState()`).
    virtual void DispatchDidPushStateWithinPage() noexcept
    {
    }

    /// @brief The document replaced its history entry (`replaceState()`).
    virtual void DispatchDidReplaceStateWithinPage() noexcept
    {
    }

    /// @brief The frame went to a history entry within its document (`popstate`).
    virtual void DispatchDidPopStateWithinPage() noexcept
    {
    }
  };

  /// @brief The loader client of a frame that has no host: it ignores every navigation.
  class EmptyFrameLoaderClient final : public LocalFrameLoaderClient
  {
  public:
    KRYS_NODISCARD PolicyAction DispatchDecidePolicyForNavigationAction(const NavigationAction &,
                                                                        const ResourceRequest &,
                                                                        const FormSubmission *) noexcept final
    {
      return PolicyAction::Ignore;
    }

    KRYS_NODISCARD PolicyAction DispatchDecidePolicyForNewWindowAction(const NavigationAction &,
                                                                       const ResourceRequest &,
                                                                       const FormSubmission *,
                                                                       dom::DOMStringView) noexcept final
    {
      return PolicyAction::Ignore;
    }
  };
}
