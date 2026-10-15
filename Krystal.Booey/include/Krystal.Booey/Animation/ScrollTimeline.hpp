#pragma once

#include "Krystal.Booey/Animation/ScrollAxis.hpp"
#include "Krystal.Booey/CSS/Types/CSSOMString.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo
{
  // FIXME: Placeholder for the scroll-driven animation timeline, which comes with web animations. Computed
  // styles create and hold timelines, so this only keeps the name and axis it was created with until
  // animations are ported.
  class ScrollTimeline : public RefCounted<ScrollTimeline>
  {
  private:
    css::CSSOMStringAtom _name;
    ScrollAxis _axis;

  public:
    virtual ~ScrollTimeline() noexcept = default;

    KRYS_NODISCARD static Ref<ScrollTimeline> Create(const css::CSSOMStringAtom &name,
                                                     ScrollAxis axis) noexcept
    {
      return AdoptRef(*new ScrollTimeline(name, axis));
    }

    KRYS_NODISCARD const css::CSSOMStringAtom &GetName() const noexcept
    {
      return _name;
    }

    KRYS_NODISCARD ScrollAxis Axis() const noexcept
    {
      return _axis;
    }

  protected:
    ScrollTimeline(const css::CSSOMStringAtom &name, ScrollAxis axis) noexcept : _name {name}, _axis {axis}
    {
    }
  };
}
