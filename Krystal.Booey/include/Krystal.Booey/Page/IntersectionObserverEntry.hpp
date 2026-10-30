#pragma once

#include "Krystal.Booey/DOM/Dicts/DOMRectInit.hpp"
#include "Krystal.Booey/DOM/DOMRectReadOnly.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Types/Maybe.hpp"

namespace krys::boo
{
  /// @brief A change in how much of an observed element intersects its intersection observer's root,
  /// delivered to the observer's callback.
  /// @see https://w3c.github.io/IntersectionObserver/#intersection-observer-entry
  class IntersectionObserverEntry : public RefCounted<IntersectionObserverEntry>
  {
  public:
    /// @see https://w3c.github.io/IntersectionObserver/#dictdef-intersectionobserverentryinit
    struct Init
    {
      double Time {0};
      Maybe<dom::DOMRectInit> RootBounds;
      dom::DOMRectInit BoundingClientRect;
      dom::DOMRectInit IntersectionRect;
      double IntersectionRatio {0};
      RefPtr<dom::Element> Target;
      bool IsIntersecting {false};
    };

  private:
    double _time {0};
    RefPtr<dom::DOMRectReadOnly> _rootBounds;
    RefPtr<dom::DOMRectReadOnly> _boundingClientRect;
    RefPtr<dom::DOMRectReadOnly> _intersectionRect;
    double _intersectionRatio {0};
    RefPtr<dom::Element> _target;
    bool _isIntersecting {false};

    explicit IntersectionObserverEntry(const Init &init) noexcept;

  public:
    KRYS_NODISCARD static Ref<IntersectionObserverEntry> Create(const Init &init) noexcept
    {
      return AdoptRef(*new IntersectionObserverEntry(init));
    }

    /// @see https://w3c.github.io/IntersectionObserver/#dom-intersectionobserverentry-time
    KRYS_NODISCARD double Time() const noexcept
    {
      return _time;
    }

    /// @see https://w3c.github.io/IntersectionObserver/#dom-intersectionobserverentry-rootbounds
    KRYS_NODISCARD dom::DOMRectReadOnly *RootBounds() const noexcept
    {
      return _rootBounds.get();
    }

    /// @see https://w3c.github.io/IntersectionObserver/#dom-intersectionobserverentry-boundingclientrect
    KRYS_NODISCARD dom::DOMRectReadOnly *BoundingClientRect() const noexcept
    {
      return _boundingClientRect.get();
    }

    /// @see https://w3c.github.io/IntersectionObserver/#dom-intersectionobserverentry-intersectionrect
    KRYS_NODISCARD dom::DOMRectReadOnly *IntersectionRect() const noexcept
    {
      return _intersectionRect.get();
    }

    /// @see https://w3c.github.io/IntersectionObserver/#dom-intersectionobserverentry-target
    KRYS_NODISCARD dom::Element *Target() const noexcept
    {
      return _target.get();
    }

    /// @see https://w3c.github.io/IntersectionObserver/#dom-intersectionobserverentry-isintersecting
    KRYS_NODISCARD bool IsIntersecting() const noexcept
    {
      return _isIntersecting;
    }

    /// @see https://w3c.github.io/IntersectionObserver/#dom-intersectionobserverentry-intersectionratio
    KRYS_NODISCARD double IntersectionRatio() const noexcept
    {
      return _intersectionRatio;
    }
  };
}
