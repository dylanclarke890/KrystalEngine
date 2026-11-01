#include "Krystal.Booey/HTML/LazyLoadImageObserver.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/HTML/HTMLImageElement.hpp"
#include "Krystal.Booey/Page/IntersectionObserverEntry.hpp"

namespace krys::boo::html
{
  namespace
  {
    /// @brief The lazy load intersection observer's callback, from step 2 of starting to intersection-observe
    /// a lazy loading element. Only images load lazily, and an image's lazy load resumption steps are
    /// `LoadDeferredImage`.
    void LoadIntersectingImages(const List<Ref<IntersectionObserverEntry>> &entries,
                                IntersectionObserver &) noexcept
    {
      krys_debug_assert(!entries.empty());

      // 2a. For each entry in entries:
      for (auto &entry : entries)
      {
        // 2a.i. Let resumptionSteps be null.
        // 2a.ii. If entry.isIntersecting is true, then set resumptionSteps to entry.target's lazy load
        //        resumption steps.
        // 2a.iii. Let posterResumptionSteps be null.
        // 2a.iv. If entry.isIntersecting is true and entry.target is a video element, then set
        //        posterResumptionSteps to the video element's poster lazy load resumption steps. (Skipped: no
        //        video playback.)
        // 2a.v. If resumptionSteps is null and posterResumptionSteps is null, then continue.
        if (!entry->IsIntersecting())
        {
          continue;
        }
        if (RefPtr element = ShareRefPtr(DynamicDowncast<HTMLImageElement>(entry->Target())))
        {
          // 2a.vi. Stop intersection-observing a lazy loading element for entry.target.
          LazyLoadImageObserver::Unobserve(*element, element->NodeDocument());

          // 2a.vii. If resumptionSteps is not null:
          // 2a.vii.A. Set entry.target's lazy load resumption steps to null.
          // 2a.vii.B. Invoke resumptionSteps.
          element->LoadDeferredImage();

          // 2a.viii. If posterResumptionSteps is not null: (Skipped: no video playback.)
        }
      }
    }
  }

  void LazyLoadImageObserver::Observe(dom::Element &element) noexcept
  {
    // 1. Let doc be element's node document.
    Ref document = ShareRef(element.NodeDocument());

    // 2. If doc's lazy load intersection observer is null, set it to a new IntersectionObserver instance,
    //    initialized as follows: (GetIntersectionObserver.)
    auto *intersectionObserver = document->GetLazyLoadImageObserver().GetIntersectionObserver(*document);
    if (!intersectionObserver)
    {
      return;
    }

    // 3. Call doc's lazy load intersection observer's observe method with element as the argument.
    intersectionObserver->Observe(element);
  }

  void LazyLoadImageObserver::Unobserve(dom::Element &element, dom::Document &document) noexcept
  {
    // 1. Let doc be element's node document.
    // 2. Assert: doc's lazy load intersection observer is not null.
    // The document is given, since an image that moved to another document stops being observed by its old
    // one, which may not have made an observer.
    // 3. Call doc's lazy load intersection observer's unobserve method with element as the argument.
    if (auto &observer = document.GetLazyLoadImageObserver()._observer)
    {
      observer->Unobserve(element);
    }
  }

  IntersectionObserver *LazyLoadImageObserver::GetIntersectionObserver(dom::Document &document) noexcept
  {
    if (!_observer)
    {
      // The callback is LoadIntersectingImages, and the options are «[ "scrollMargin" → lazy load scroll
      // margin ]»; the lazy load scroll margin is 100% of the viewport.
      auto observer = IntersectionObserver::Create(document, &LoadIntersectingImages,
                                                   {.RootMargin = u8"", .ScrollMargin = u8"100%"});
      if (observer.HasException())
      {
        return nullptr;
      }
      _observer = observer.ReleaseValue();
    }
    return _observer.get();
  }

  bool LazyLoadImageObserver::IsObserved(dom::Element &element) const noexcept
  {
    return _observer && _observer->IsObserving(element);
  }
}
