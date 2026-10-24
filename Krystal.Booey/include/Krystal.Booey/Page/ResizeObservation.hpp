#pragma once

#include "Krystal.Booey/Graphics/FloatRect.hpp"
#include "Krystal.Booey/Graphics/FloatSize.hpp"
#include "Krystal.Booey/Graphics/LayoutPoint.hpp"
#include "Krystal.Booey/Graphics/LayoutSize.hpp"
#include "Krystal.Booey/Page/ResizeObserverBoxOptions.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"

namespace krys::boo::dom
{
  class Element;
}

namespace krys::boo
{
  /// @brief A resize observer's observation of one element: the box it observes and the sizes it last
  /// reported.
  /// @see https://drafts.csswg.org/resize-observer/#resizeobservation
  class ResizeObservation : public RefCounted<ResizeObservation>
  {
  public:
    struct BoxSizes
    {
      LayoutSize ContentBoxSize;
      LayoutSize ContentBoxLogicalSize;
      LayoutSize BorderBoxLogicalSize;
    };

  private:
    WeakPtr<dom::Element> _target;
    BoxSizes _lastObservationSizes;
    ResizeObserverBoxOptions _observedBox;

    ResizeObservation(dom::Element &target, ResizeObserverBoxOptions observedBox) noexcept;

  public:
    KRYS_NODISCARD static Ref<ResizeObservation> Create(dom::Element &target,
                                                        ResizeObserverBoxOptions observedBox) noexcept;
    ~ResizeObservation() noexcept;

    /// @brief The target's current sizes, if the observed box's size differs from the last reported one.
    /// @see https://drafts.csswg.org/resize-observer/#dom-resizeobservation-isactive
    KRYS_NODISCARD Maybe<BoxSizes> ElementSizeChanged() const noexcept;
    void UpdateObservationSize(const BoxSizes &boxSizes) noexcept;
    /// @brief Forgets the last reported sizes, so the next gathering reports the target.
    void ResetObservationSize() noexcept;

    KRYS_NODISCARD FloatRect ComputeContentRect() const noexcept;
    KRYS_NODISCARD FloatSize BorderBoxSize() const noexcept;
    KRYS_NODISCARD FloatSize ContentBoxSize() const noexcept;
    KRYS_NODISCARD FloatSize SnappedContentBoxSize() const noexcept;

    KRYS_NODISCARD dom::Element *Target() const noexcept
    {
      return _target.get();
    }
    KRYS_NODISCARD ResizeObserverBoxOptions ObservedBox() const noexcept
    {
      return _observedBox;
    }

    /// @see https://drafts.csswg.org/resize-observer/#calculate-depth-for-node
    KRYS_NODISCARD size_t TargetElementDepth() const noexcept;

  private:
    KRYS_NODISCARD Maybe<BoxSizes> ComputeObservedSizes() const noexcept;
    KRYS_NODISCARD LayoutPoint ComputeTargetLocation() const noexcept;
  };
}
