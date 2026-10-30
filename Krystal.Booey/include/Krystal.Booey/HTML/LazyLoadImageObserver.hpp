#pragma once

#include "Krystal.Booey/Page/IntersectionObserver.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::dom
{
  class Document;
  class Element;
}

namespace krys::boo::html
{
  /// @brief Loads a document's lazily loaded images (`loading=lazy`) once they intersect the viewport, with
  /// an intersection observer the document makes on first use.
  /// @see https://html.spec.whatwg.org/#lazy-load-intersection-observer
  class LazyLoadImageObserver
  {
    KRYS_NON_COPY_MOVABLE(LazyLoadImageObserver);

  private:
    RefPtr<IntersectionObserver> _observer;

  public:
    LazyLoadImageObserver() noexcept = default;

    /// @brief Starts observing @p element, the image of a deferred load, with its document's observer.
    static void Observe(dom::Element &element) noexcept;
    /// @brief Stops observing @p element with @p document's observer.
    static void Unobserve(dom::Element &element, dom::Document &document) noexcept;

    KRYS_NODISCARD bool IsObserved(dom::Element &element) const noexcept;

  private:
    KRYS_NODISCARD IntersectionObserver *GetIntersectionObserver(dom::Document &document) noexcept;
  };
}
