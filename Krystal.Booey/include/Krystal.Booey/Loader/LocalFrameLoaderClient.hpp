#pragma once

#include "Krystal.Booey/DOM/Types/DOMString.hpp"
#include "Krystal.Booey/Loader/FrameLoaderTypes.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"

namespace krys::boo
{
  class FormSubmission;
  class NavigationAction;
  class ResourceRequest;

  /// @brief What a frame's loader asks of its host: whether to navigate, and what happened.
  /// @note The host answers each navigation as it's asked, rather than later.
  // FIXME: Document loading (provisional loads, commits, the load events, errors), history, downloads and
  // the rest of the client, once there's document loading (B3).
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

    /// @brief The frame navigated within its document (to a fragment).
    virtual void DispatchDidNavigateWithinPage() noexcept
    {
    }

    /// @brief The frame's URL changed within its document: its fragment changed.
    virtual void DispatchDidChangeLocationWithinPage() noexcept
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
