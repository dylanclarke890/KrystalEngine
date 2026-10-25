#include "Krystal.Booey/HTML/LazyLoadImageObserver.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/HTML/HTMLImageElement.hpp"
#include "Krystal.Booey/Page/IntersectionObserverEntry.hpp"

namespace krys::boo::html
{
  namespace
  {
    void LoadIntersectingImages(const List<Ref<IntersectionObserverEntry>> &entries,
                                IntersectionObserver &) noexcept
    {
      krys_debug_assert(!entries.empty());

      for (auto &entry : entries)
      {
        if (!entry->IsIntersecting())
        {
          continue;
        }
        if (RefPtr element = ShareRefPtr(DynamicDowncast<HTMLImageElement>(entry->Target())))
        {
          element->LoadDeferredImage();
          LazyLoadImageObserver::Unobserve(*element, element->NodeDocument());
        }
      }
    }
  }

  void LazyLoadImageObserver::Observe(dom::Element &element) noexcept
  {
    Ref document = ShareRef(element.NodeDocument());
    auto *intersectionObserver = document->GetLazyLoadImageObserver().GetIntersectionObserver(*document);
    if (!intersectionObserver)
    {
      return;
    }
    intersectionObserver->Observe(element);
  }

  void LazyLoadImageObserver::Unobserve(dom::Element &element, dom::Document &document) noexcept
  {
    if (auto &observer = document.GetLazyLoadImageObserver()._observer)
    {
      observer->Unobserve(element);
    }
  }

  IntersectionObserver *LazyLoadImageObserver::GetIntersectionObserver(dom::Document &document) noexcept
  {
    if (!_observer)
    {
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
