#pragma once

#include "Krystal.Booey/Animation/ScrollTimeline.hpp"
#include "Krystal.Booey/Style/Values/ScrollAnimations/ViewTimelineInsets.hpp"

namespace krys::boo
{
  // FIXME: Placeholder for the view progress timeline, which comes with web animations (see
  // `ScrollTimeline`).
  class ViewTimeline final : public ScrollTimeline
  {
  private:
    style::ViewTimelineInsetItem _insets;

  public:
    KRYS_NODISCARD static Ref<ViewTimeline> Create(const css::CSSOMStringAtom &name, ScrollAxis axis,
                                                   const style::ViewTimelineInsetItem &insets) noexcept
    {
      return AdoptRef(*new ViewTimeline(name, axis, insets));
    }

    KRYS_NODISCARD const style::ViewTimelineInsetItem &Insets() const noexcept
    {
      return _insets;
    }

  private:
    ViewTimeline(const css::CSSOMStringAtom &name, ScrollAxis axis,
                 const style::ViewTimelineInsetItem &insets) noexcept
        : ScrollTimeline {name, axis}, _insets {insets}
    {
    }
  };
}
