#pragma once

#include "Krystal.Booey/DOM/Types/DOMString.hpp"
#include "Krystal.Booey/Loader/FrameLoaderTypes.hpp"
#include "Krystal.Booey/URL/URL.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"

namespace krys::boo
{
  class FormSubmission;
  class NavigationAction;
  class ResourceError;
  class ResourceRequest;

  /// @brief What a frame's loader tells its host: the navigations the page asks for, and what happened to
  /// the loads the host started.
  // FIXME: Downloads, the progress and the rest of the client, once there are those.
  class LocalFrameLoaderClient
  {
    KRYS_NON_COPY_MOVABLE(LocalFrameLoaderClient);

  public:
    LocalFrameLoaderClient() noexcept = default;
    virtual ~LocalFrameLoaderClient() noexcept = default;

    /// @brief The page asks the frame to navigate to @p request (a link followed or a form submitted), which
    /// @p action started. Booey doesn't load it: the host decides, and loads it with `FrameLoader::Load` if
    /// it wants.
    /// @param formSubmission The form submitted, for a form submission.
    virtual void DispatchNavigationRequested(const NavigationAction &action, const ResourceRequest &request,
                                             const FormSubmission *formSubmission) noexcept
    {
      (void)action;
      (void)request;
      (void)formSubmission;
    }

    /// @brief The page asks for a new browsing context named @p frameName (`_blank` or a name no frame has)
    /// to navigate to @p request; the host opens one if it wants.
    virtual void DispatchNewWindowRequested(const NavigationAction &action, const ResourceRequest &request,
                                            const FormSubmission *formSubmission,
                                            dom::DOMStringView frameName) noexcept
    {
      (void)action;
      (void)request;
      (void)formSubmission;
      (void)frameName;
    }

    /// @brief A document started loading in the frame (provisionally: the frame shows its old document).
    virtual void DispatchDidStartProvisionalLoad() noexcept
    {
    }

    /// @brief The provisional load failed (or was stopped): the frame keeps its document.
    virtual void DispatchDidFailProvisionalLoad(const ResourceError &error) noexcept
    {
      (void)error;
    }

    /// @brief The loading document is the frame's now, made and not yet parsed.
    virtual void DispatchDidCommitLoad() noexcept
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

    /// @brief The frame navigated within its document: to a fragment of its URL.
    virtual void DispatchDidNavigateWithinPage() noexcept
    {
    }
  };

  /// @brief The loader client of a frame that has no host: it ignores every navigation.
  class EmptyFrameLoaderClient final : public LocalFrameLoaderClient
  {
  };
}
