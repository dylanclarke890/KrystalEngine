#pragma once

#include "Krystal.Booey/Graphics/DoublePoint.hpp"
#include "Krystal.Booey/Graphics/DoubleSize.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo
{
  class LocalFrame;
}

namespace krys::boo::dom
{
  class EventTarget;

  /// @brief A touch on the screen, as a touch event reports it: where it is, the element it began on and the
  /// size and pressure of its contact.
  /// @see https://w3c.github.io/touch-events/#touch-interface
  class Touch : public RefCounted<Touch>
  {
  private:
    RefPtr<EventTarget> _target;
    int32 _identifier {0};
    // Position relative to the viewport in CSS px.
    DoublePoint _clientPosition;
    // Position relative to the screen in DIPs.
    DoublePoint _screenPosition;
    // Position relative to the page in CSS px.
    DoublePoint _pagePosition;
    // Radius in CSS px.
    DoubleSize _radius;
    float _rotationAngle {0};
    // Twist is stored so that it can be exposed for pointer events.
    double _twist {0};
    float _force {0};
    DoublePoint _absoluteLocation;

  public:
    KRYS_NODISCARD static Ref<Touch> Create(LocalFrame *frame, EventTarget *target, int32 identifier,
                                            const DoublePoint &screenPos, const DoublePoint &pagePos,
                                            const DoubleSize &radius, float rotationAngle, float force,
                                            double twist = 0) noexcept;

    ~Touch() noexcept;

    /// @see https://w3c.github.io/touch-events/#dom-touch-target
    KRYS_NODISCARD EventTarget *Target() const noexcept
    {
      return _target.get();
    }

    /// @see https://w3c.github.io/touch-events/#dom-touch-identifier
    KRYS_NODISCARD int32 Identifier() const noexcept
    {
      return _identifier;
    }

    /// @see https://w3c.github.io/touch-events/#dom-touch-clientx
    KRYS_NODISCARD double ClientX() const noexcept
    {
      return _clientPosition.X();
    }

    /// @see https://w3c.github.io/touch-events/#dom-touch-clienty
    KRYS_NODISCARD double ClientY() const noexcept
    {
      return _clientPosition.Y();
    }

    /// @see https://w3c.github.io/touch-events/#dom-touch-screenx
    KRYS_NODISCARD double ScreenX() const noexcept
    {
      return _screenPosition.X();
    }

    /// @see https://w3c.github.io/touch-events/#dom-touch-screeny
    KRYS_NODISCARD double ScreenY() const noexcept
    {
      return _screenPosition.Y();
    }

    /// @see https://w3c.github.io/touch-events/#dom-touch-pagex
    KRYS_NODISCARD double PageX() const noexcept
    {
      return _pagePosition.X();
    }

    /// @see https://w3c.github.io/touch-events/#dom-touch-pagey
    KRYS_NODISCARD double PageY() const noexcept
    {
      return _pagePosition.Y();
    }

    /// @see https://w3c.github.io/touch-events/#dom-touch-radiusx
    KRYS_NODISCARD double RadiusX() const noexcept
    {
      return _radius.Width();
    }

    /// @see https://w3c.github.io/touch-events/#dom-touch-radiusy
    KRYS_NODISCARD double RadiusY() const noexcept
    {
      return _radius.Height();
    }

    /// @see https://w3c.github.io/touch-events/#dom-touch-rotationangle
    KRYS_NODISCARD float RotationAngle() const noexcept
    {
      return _rotationAngle;
    }

    KRYS_NODISCARD double Twist() const noexcept
    {
      return _twist;
    }

    /// @see https://w3c.github.io/touch-events/#dom-touch-force
    KRYS_NODISCARD float Force() const noexcept
    {
      return _force;
    }

    KRYS_NODISCARD const DoublePoint &AbsoluteLocation() const noexcept
    {
      return _absoluteLocation;
    }

    KRYS_NODISCARD Ref<Touch> CloneWithNewTarget(EventTarget *eventTarget) const noexcept;

  private:
    Touch(LocalFrame *frame, EventTarget *target, int32 identifier, const DoublePoint &screenPosition,
          const DoublePoint &pagePosition, const DoubleSize &radius, float rotationAngle, double twist,
          float force) noexcept;
    Touch(EventTarget *target, int32 identifier, const DoublePoint &clientPosition,
          const DoublePoint &screenPosition, const DoublePoint &pagePosition, const DoubleSize &radius,
          float rotationAngle, double twist, float force, const DoublePoint &absoluteLocation) noexcept;
  };
}
