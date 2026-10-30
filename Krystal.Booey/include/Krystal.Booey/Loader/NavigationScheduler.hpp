#pragma once

#include "Krystal.Booey/DOM/Types/DOMString.hpp"
#include "Krystal.Booey/Loader/FrameLoaderTypes.hpp"
#include "Krystal.Booey/Platform/Timer.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Types/Func.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"

namespace krys::boo::dom
{
  class Document;
}

namespace krys::boo::url
{
  class URL;
}

namespace krys::boo
{
  class FormSubmission;
  class LocalFrame;

  /// @brief Whether a scheduled traversal by key happened.
  enum class ScheduleHistoryNavigationResult : bool
  {
    Completed,
    Aborted
  };
  class ScheduledNavigation;

  /// @brief Navigations a frame does later: redirects (`<meta http-equiv=refresh>`), location changes
  /// (`Location`), form submissions and refreshes, one at a time (a newer one replaces an older one), on a
  /// timer.
  class NavigationScheduler
  {
    KRYS_NON_COPY_MOVABLE(NavigationScheduler);

  private:
    LocalFrame &_frame;
    Timer _timer;
    UniquePtr<ScheduledNavigation> _redirect;

  public:
    explicit NavigationScheduler(LocalFrame &frame) noexcept;
    ~NavigationScheduler() noexcept;

    /// @brief Whether a location change to another document is scheduled.
    KRYS_NODISCARD bool LocationChangePending() const noexcept;

    /// @brief Whether a navigation is scheduled to happen at once.
    KRYS_NODISCARD bool HasQueuedNavigation() const noexcept;

    /// @brief Schedules a redirect to @p url after @p delay seconds, which starts once the document has
    /// loaded; a later redirect with a longer delay is ignored.
    void ScheduleRedirect(dom::Document &initiatingDocument, double delay, const url::URL &url,
                          IsMetaRefresh isMetaRefresh) noexcept;

    /// @brief Schedules a navigation to @p url, at once; one to a fragment of the document's URL happens now.
    void ScheduleLocationChange(dom::Document &initiatingDocument, const url::URL &url,
                                NavigationHistoryBehavior historyHandling) noexcept;

    /// @brief Schedules @p submission's navigation, at once.
    void ScheduleFormSubmission(Ref<FormSubmission> &&submission) noexcept;

    /// @brief Schedules loading the document's URL again, at once.
    void ScheduleRefresh(dom::Document &initiatingDocument) noexcept;

    /// @brief Schedules going @p steps entries back (if negative) or forward in the session history, at once;
    /// cancels the scheduled navigation if there's no such entry.
    void ScheduleHistoryNavigation(int32 steps) noexcept;

    /// @brief Schedules going to the Navigation API's entry whose key is @p key, at once; @p
    /// completionHandler hears whether it happened.
    void ScheduleHistoryNavigationByKey(
      dom::DOMString &&key, Func<void(ScheduleHistoryNavigationResult)> &&completionHandler) noexcept;

    /// @brief Starts the scheduled navigation's timer, if it can start now (a redirect, once the document has
    /// loaded).
    void StartTimer() noexcept;

    /// @brief Cancels the scheduled navigation, if any.
    void Cancel() noexcept;

  private:
    void Schedule(UniquePtr<ScheduledNavigation> &&redirect) noexcept;
    void TimerFired() noexcept;
  };
}
