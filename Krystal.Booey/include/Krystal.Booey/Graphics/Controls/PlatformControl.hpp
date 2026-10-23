#pragma once

#include "Krystal.Booey/Graphics/Controls/ControlStyle.hpp"
#include "Krystal.Booey/Graphics/FloatRect.hpp"
#include "Krystal.Core/Attributes.hpp"

namespace krys::boo
{
  class ControlPart;
  class FloatRoundedRect;
  class GraphicsContext;

  /// @brief A theme's drawing of a control part: sizes it and draws it.
  class PlatformControl
  {
  protected:
    ControlPart &_owningPart;

  public:
    explicit PlatformControl(ControlPart &owningPart) noexcept : _owningPart(owningPart)
    {
    }

    virtual ~PlatformControl() noexcept = default;

    virtual void SetFocusRingClipRect(const FloatRect &) noexcept
    {
    }

    virtual void UpdateCellStates(const FloatRect &, const ControlStyle &) noexcept
    {
    }

    KRYS_NODISCARD virtual FloatSize SizeForBounds(const FloatRect &bounds,
                                                   const ControlStyle &) const noexcept
    {
      return bounds.Size();
    }

    KRYS_NODISCARD virtual FloatRect RectForBounds(const FloatRect &bounds,
                                                   const ControlStyle &) const noexcept
    {
      return bounds;
    }

    virtual void Draw(GraphicsContext &, const FloatRoundedRect &, float, const ControlStyle &) noexcept
    {
    }
  };
}
