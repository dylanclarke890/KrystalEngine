#include "Krystal.Booey/Loader/NavigationScheduler.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/Event.hpp"
#include "Krystal.Booey/History/BackForwardController.hpp"
#include "Krystal.Booey/History/HistoryItem.hpp"
#include "Krystal.Booey/HTML/HTMLFormControlElement.hpp"
#include "Krystal.Booey/HTML/HTMLFormElement.hpp"
#include "Krystal.Booey/Loader/DocumentLoader.hpp"
#include "Krystal.Booey/Loader/FormSubmission.hpp"
#include "Krystal.Booey/Loader/FrameLoader.hpp"
#include "Krystal.Booey/Loader/FrameLoadRequest.hpp"
#include "Krystal.Booey/Page/LocalDOMWindow.hpp"
#include "Krystal.Booey/Page/LocalFrame.hpp"
#include "Krystal.Booey/Page/Navigation.hpp"
#include "Krystal.Booey/Page/Page.hpp"
#include "Krystal.Booey/URL/URL.hpp"
#include <chrono>
#include <climits>

namespace krys::boo
{
  /// @brief A navigation a frame does later.
  class ScheduledNavigation
  {
    KRYS_NON_COPY_MOVABLE(ScheduledNavigation);

  private:
    double _delay;
    bool _wasDuringLoad;
    bool _isLocationChange;

  public:
    ScheduledNavigation(double delay, bool wasDuringLoad, bool isLocationChange) noexcept
        : _delay(delay), _wasDuringLoad(wasDuringLoad), _isLocationChange(isLocationChange)
    {
    }
    virtual ~ScheduledNavigation() noexcept = default;

    virtual void Fire(LocalFrame &frame) noexcept = 0;

    KRYS_NODISCARD virtual bool ShouldStartTimer(LocalFrame &) const noexcept
    {
      return true;
    }

    KRYS_NODISCARD virtual bool IsSameDocumentNavigation(LocalFrame &) const noexcept
    {
      return false;
    }

    KRYS_NODISCARD double Delay() const noexcept
    {
      return _delay;
    }

    KRYS_NODISCARD bool WasDuringLoad() const noexcept
    {
      return _wasDuringLoad;
    }

    KRYS_NODISCARD bool IsLocationChange() const noexcept
    {
      return _isLocationChange;
    }
  };

  namespace
  {
    /// @brief A navigation to a URL, which a document started.
    class ScheduledURLNavigation : public ScheduledNavigation
    {
    protected:
      Ref<dom::Document> _initiatingDocument;
      url::URL _url;

      ScheduledURLNavigation(dom::Document &initiatingDocument, double delay, const url::URL &url,
                             bool duringLoad, bool isLocationChange) noexcept
          : ScheduledNavigation(delay, duringLoad, isLocationChange),
            _initiatingDocument(ShareRef(initiatingDocument)), _url(url)
      {
      }

    public:
      KRYS_NODISCARD bool IsSameDocumentNavigation(LocalFrame &) const noexcept final
      {
        return url::EqualIgnoringFragmentIdentifier(_initiatingDocument->GetURL(), _url);
      }
    };

    /// @brief A redirect: `<meta http-equiv=refresh>`.
    class ScheduledRedirect final : public ScheduledURLNavigation
    {
    public:
      ScheduledRedirect(dom::Document &initiatingDocument, double delay, const url::URL &url) noexcept
          : ScheduledURLNavigation(initiatingDocument, delay, url, false, false)
      {
      }

      KRYS_NODISCARD bool ShouldStartTimer(LocalFrame &frame) const noexcept override
      {
        // A redirect waits for the document to finish loading.
        return frame.Loader().IsComplete();
      }

      void Fire(LocalFrame &frame) noexcept override
      {
        // FIXME: The sandboxed automatic features flag, once there's sandboxing.
        // A redirect to the document's URL reloads it; one within a second replaces its history item.
        bool refresh = url::EqualIgnoringFragmentIdentifier(frame.Document()->GetURL(), _url);
        bool lockBackForwardList = Delay() <= 1;
        frame.Loader().ClientRedirected(lockBackForwardList);
        FrameLoadRequest request(ShareRef(*_initiatingDocument), ResourceRequest(url::URL(_url)),
                                 dom::DOMStringAtom(u8"_self"));
        request.SetNavigationHistoryBehavior(refresh               ? NavigationHistoryBehavior::Reload
                                             : lockBackForwardList ? NavigationHistoryBehavior::Replace
                                                                   : NavigationHistoryBehavior::Auto);
        if (refresh)
        {
          frame.Loader().LoadFrameRequest(krys::move(request), nullptr, nullptr);
          return;
        }
        frame.Loader().ChangeLocation(krys::move(request));
      }
    };

    /// @brief A location change: `location.href`, `assign()` and `replace()`.
    class ScheduledLocationChange final : public ScheduledURLNavigation
    {
    private:
      NavigationHistoryBehavior _historyBehavior;

    public:
      ScheduledLocationChange(dom::Document &initiatingDocument, const url::URL &url, bool duringLoad,
                              NavigationHistoryBehavior historyBehavior) noexcept
          : ScheduledURLNavigation(initiatingDocument, 0, url, duringLoad, true),
            _historyBehavior(historyBehavior)
      {
      }

      void Fire(LocalFrame &frame) noexcept override
      {
        frame.Loader().ClientRedirected(_historyBehavior == NavigationHistoryBehavior::Replace);
        FrameLoadRequest request(ShareRef(*_initiatingDocument), ResourceRequest(url::URL(_url)),
                                 dom::DOMStringAtom(u8"_self"));
        request.SetNavigationHistoryBehavior(_historyBehavior);
        frame.Loader().ChangeLocation(krys::move(request));
      }
    };

    /// @brief A reload: `location.reload()`.
    class ScheduledRefresh final : public ScheduledURLNavigation
    {
    public:
      ScheduledRefresh(dom::Document &initiatingDocument, const url::URL &url) noexcept
          : ScheduledURLNavigation(initiatingDocument, 0, url, false, true)
      {
      }

      void Fire(LocalFrame &frame) noexcept override
      {
        // A reload: the document's subresources are revalidated, and its history item is updated.
        FrameLoadRequest request(ShareRef(*_initiatingDocument), ResourceRequest(url::URL(_url)),
                                 dom::DOMStringAtom(u8"_self"));
        request.SetNavigationHistoryBehavior(NavigationHistoryBehavior::Reload);
        frame.Loader().LoadFrameRequest(krys::move(request), nullptr, nullptr);
      }
    };

    /// @brief A history navigation: `history.go()`, `back()` and `forward()`.
    class ScheduledHistoryNavigation final : public ScheduledNavigation
    {
    private:
      Ref<HistoryItem> _historyItem;

    public:
      explicit ScheduledHistoryNavigation(Ref<HistoryItem> &&historyItem) noexcept
          : ScheduledNavigation(0, false, true), _historyItem(krys::move(historyItem))
      {
      }

      void Fire(LocalFrame &frame) noexcept override
      {
        RefPtr<Page> page = ShareRefPtr(frame.GetPage());
        if (!page || !page->BackForward().ContainsItem(*_historyItem))
        {
          return;
        }

        // Going to the current entry reloads it.
        if (RefPtr<HistoryItem> currentItem = page->BackForward().CurrentItem();
            currentItem && currentItem->ItemID() == _historyItem->ItemID())
        {
          frame.Loader().ChangeLocation(frame.Document()->GetURL(), dom::DOMStringAtom(u8"_self"), nullptr,
                                        ReferrerPolicy::EmptyString, null, dom::DOMStringAtom::Null(),
                                        NavigationHistoryBehavior::Reload);
          return;
        }

        page->GoToItem(frame, *_historyItem, FrameLoadType::IndexedBackForward);
      }

      KRYS_NODISCARD bool IsSameDocumentNavigation(LocalFrame &frame) const noexcept override
      {
        auto *page = frame.GetPage();
        if (!page || !page->BackForward().ContainsItem(*_historyItem))
        {
          return false;
        }
        return url::EqualIgnoringFragmentIdentifier(frame.Document()->GetURL(), _historyItem->URL());
      }
    };

    /// @brief A traversal to the Navigation API's entry with a key: `navigation.traverseTo()`, `back()` and
    /// `forward()`.
    class ScheduledHistoryNavigationByKey final : public ScheduledNavigation
    {
    private:
      dom::DOMString _key;
      Func<void(ScheduleHistoryNavigationResult)> _completionHandler;

    public:
      ScheduledHistoryNavigationByKey(
        dom::DOMString &&key, Func<void(ScheduleHistoryNavigationResult)> &&completionHandler) noexcept
          : ScheduledNavigation(0, false, true), _key(krys::move(key)),
            _completionHandler(krys::move(completionHandler))
      {
      }

      ~ScheduledHistoryNavigationByKey() noexcept override
      {
        if (_completionHandler)
        {
          _completionHandler(ScheduleHistoryNavigationResult::Aborted);
        }
      }

      KRYS_NODISCARD RefPtr<HistoryItem> FindBackForwardItemByKey(LocalFrame &frame) const noexcept
      {
        auto *window = frame.Document() ? frame.Document()->Window() : nullptr;
        auto *entry = window ? window->GetNavigation().FindEntryByKey(_key) : nullptr;
        return entry ? ShareRefPtr(&entry->AssociatedHistoryItem()) : nullptr;
      }

      void Fire(LocalFrame &frame) noexcept override
      {
        RefPtr<Page> page = ShareRefPtr(frame.GetPage());
        RefPtr<HistoryItem> historyItem = page ? FindBackForwardItemByKey(frame) : nullptr;
        if (!historyItem)
        {
          std::exchange(_completionHandler, nullptr)(ScheduleHistoryNavigationResult::Aborted);
          return;
        }

        if (RefPtr<HistoryItem> currentItem = page->BackForward().CurrentItem();
            currentItem && currentItem->ItemID() == historyItem->ItemID())
        {
          frame.Loader().ChangeLocation(frame.Document()->GetURL(), dom::DOMStringAtom(u8"_self"), nullptr,
                                        ReferrerPolicy::EmptyString, null, dom::DOMStringAtom::Null(),
                                        NavigationHistoryBehavior::Reload);
          return;
        }

        auto completionHandler = std::exchange(_completionHandler, nullptr);
        auto *upcomingTraverseMethodTracker =
          frame.Document()->Window()->GetNavigation().UpcomingTraverseMethodTracker(_key);
        page->GoToItemForNavigationAPI(frame, *historyItem, FrameLoadType::IndexedBackForward, frame,
                                       upcomingTraverseMethodTracker);
        completionHandler(ScheduleHistoryNavigationResult::Completed);
      }

      KRYS_NODISCARD bool IsSameDocumentNavigation(LocalFrame &frame) const noexcept override
      {
        RefPtr<HistoryItem> historyItem = frame.GetPage() ? FindBackForwardItemByKey(frame) : nullptr;
        return historyItem
               && url::EqualIgnoringFragmentIdentifier(frame.Document()->GetURL(), historyItem->URL());
      }
    };

    /// @brief A form submission.
    class ScheduledFormSubmission final : public ScheduledNavigation
    {
    private:
      Ref<FormSubmission> _submission;

    public:
      ScheduledFormSubmission(Ref<FormSubmission> &&submission, bool duringLoad) noexcept
          : ScheduledNavigation(0, duringLoad, true), _submission(krys::move(submission))
      {
      }

      void Fire(LocalFrame &frame) noexcept override
      {
        auto *form = _submission->Form();
        auto *document = frame.Document();
        if (!document)
        {
          return;
        }
        auto &requestingDocument = form ? form->NodeDocument() : *document;

        FrameLoadRequest frameLoadRequest(ShareRef(requestingDocument), ResourceRequest(),
                                          dom::DOMStringAtom::Null());
        _submission->PopulateFrameLoadRequest(frameLoadRequest);
        frameLoadRequest.SetNavigationHistoryBehavior(NavigationHistoryBehavior::Push);
        frameLoadRequest.SetSourceElement(form);
        RefPtr<dom::Event> event = ShareRefPtr(_submission->Event());
        frame.Loader().LoadFrameRequest(krys::move(frameLoadRequest), event.get(),
                                        RefPtr<FormSubmission>(_submission));
      }
    };
  }

  NavigationScheduler::NavigationScheduler(LocalFrame &frame) noexcept
      : _frame(frame), _timer(*this, &NavigationScheduler::TimerFired)
  {
  }

  NavigationScheduler::~NavigationScheduler() noexcept = default;

  bool NavigationScheduler::LocationChangePending() const noexcept
  {
    return _redirect && _redirect->IsLocationChange() && !_redirect->IsSameDocumentNavigation(_frame);
  }

  bool NavigationScheduler::HasQueuedNavigation() const noexcept
  {
    return _redirect && !_redirect->Delay();
  }

  void NavigationScheduler::ScheduleRedirect(dom::Document &initiatingDocument, double delay,
                                             const url::URL &url, IsMetaRefresh) noexcept
  {
    if (!_frame.GetPage() || delay < 0 || delay > INT_MAX / 1'000 || url.IsEmpty())
    {
      return;
    }

    // A redirect with a longer delay than the one scheduled is ignored.
    if (!_redirect || delay <= _redirect->Delay())
    {
      Schedule(CreateUnique<ScheduledRedirect>(initiatingDocument, delay, url));
    }
  }

  void NavigationScheduler::ScheduleLocationChange(dom::Document &initiatingDocument, const url::URL &url,
                                                   NavigationHistoryBehavior historyHandling) noexcept
  {
    if (!_frame.GetPage())
    {
      return;
    }

    // If the URL we're going to navigate to is the same as the current one, except for the fragment part,
    // we don't need to schedule the location change.
    auto *document = _frame.Document();
    if (url.HasFragmentIdentifier() && document
        && url::EqualIgnoringFragmentIdentifier(document->GetURL(), url))
    {
      FrameLoadRequest request(ShareRef(initiatingDocument), ResourceRequest(url::URL(url)),
                               dom::DOMStringAtom(u8"_self"));
      request.SetNavigationHistoryBehavior(historyHandling);
      _frame.Loader().ChangeLocation(krys::move(request));
      return;
    }

    bool duringLoad =
      _frame.Loader().State() == FrameState::Provisional && !_frame.Loader().GetDocumentLoader();
    Schedule(CreateUnique<ScheduledLocationChange>(initiatingDocument, url, duringLoad, historyHandling));
  }

  void NavigationScheduler::ScheduleFormSubmission(Ref<FormSubmission> &&submission) noexcept
  {
    krys_debug_assert(_frame.GetPage());
    bool duringLoad =
      _frame.Loader().State() == FrameState::Provisional && !_frame.Loader().GetDocumentLoader();
    Schedule(CreateUnique<ScheduledFormSubmission>(krys::move(submission), duringLoad));
  }

  void NavigationScheduler::ScheduleRefresh(dom::Document &initiatingDocument) noexcept
  {
    auto *document = _frame.Document();
    if (!_frame.GetPage() || !document || document->GetURL().IsEmpty())
    {
      return;
    }
    Schedule(CreateUnique<ScheduledRefresh>(initiatingDocument, document->GetURL()));
  }

  void NavigationScheduler::ScheduleHistoryNavigationByKey(
    dom::DOMString &&key, Func<void(ScheduleHistoryNavigationResult)> &&completionHandler) noexcept
  {
    if (!_frame.GetPage())
    {
      completionHandler(ScheduleHistoryNavigationResult::Aborted);
      return;
    }
    Schedule(CreateUnique<ScheduledHistoryNavigationByKey>(krys::move(key), krys::move(completionHandler)));
  }

  void NavigationScheduler::ScheduleHistoryNavigation(int32 steps) noexcept
  {
    auto *page = _frame.GetPage();
    if (!page)
    {
      return;
    }

    // Invalid history navigations (such as history.forward() during a new load) have the side effect of
    // cancelling any scheduled redirects. We also avoid the possibility of cancelling the current load by
    // avoiding the scheduled redirection altogether.
    auto &backForward = page->BackForward();
    if ((steps > 0 && static_cast<uint32>(steps) > backForward.ForwardCount())
        || (steps < 0 && static_cast<uint32>(-steps) > backForward.BackCount()))
    {
      Cancel();
      return;
    }

    RefPtr<HistoryItem> historyItem = backForward.ItemAtIndex(steps);
    if (!historyItem)
    {
      Cancel();
      return;
    }

    // In all other cases, schedule the history traversal to occur asynchronously.
    Schedule(CreateUnique<ScheduledHistoryNavigation>(ShareRef(*historyItem)));
  }

  void NavigationScheduler::Schedule(UniquePtr<ScheduledNavigation> &&redirect) noexcept
  {
    // If a redirect was scheduled during a load, then stop the current load.
    if (redirect->WasDuringLoad())
    {
      _frame.Loader().StopAllLoaders();
    }

    Cancel();
    _redirect = krys::move(redirect);
    StartTimer();
  }

  void NavigationScheduler::StartTimer() noexcept
  {
    if (!_redirect || _timer.IsActive() || !_redirect->ShouldStartTimer(_frame))
    {
      return;
    }
    _timer.StartOneShot(
      std::chrono::duration_cast<MonotonicTime::duration>(std::chrono::duration<double>(_redirect->Delay())));
  }

  void NavigationScheduler::Cancel() noexcept
  {
    _timer.Stop();
    _redirect = nullptr;
  }

  void NavigationScheduler::TimerFired() noexcept
  {
    Ref<LocalFrame> frame = ShareRef(_frame);
    if (!frame->GetPage())
    {
      return;
    }
    auto redirect = krys::move(_redirect);
    redirect->Fire(*frame);
  }
}
