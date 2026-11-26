#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Geometry/Point.hpp"
#include "Krystal.Core/Geometry/Size.hpp"
#include "Krystal.Core/Numeric.hpp"

namespace krys::web
{
  /// @brief A touch on the host's touch screen, as one of a touch event's points.
  class PlatformTouchPoint
  {
  public:
    /// @brief What happened to a touch since the last touch event.
    enum State : uint8
    {
      /// @brief Lifted from the screen.
      TouchReleased,
      /// @brief Put down on the screen.
      TouchPressed,
      TouchMoved,
      /// @brief Unchanged: it's in the event because every touch on the screen is.
      TouchStationary,
      /// @brief Ended without being lifted, when the host says it was interrupted.
      TouchCancelled,
      /// @brief The number of states, not a state; it stays last.
      TouchStateEnd,
    };

  protected:
    uint32 _id {0};
    State _state {TouchReleased};
    PointD _screenPos;
    PointD _pos;
    SizeD _radius;
    float _rotationAngle {0};
    float _twist {0};
    float _force {0};

  public:
    PlatformTouchPoint() noexcept = default;

    /// @param id Identifies the touch while it's down: the host's touches on the screen have distinct ids,
    /// which a later touch may reuse.
    /// @param screenPos The point in the screen's coordinates.
    /// @param pos The point in the view's coordinates.
    /// @param radius Half the contact's width and height, in the view's coordinates.
    /// @param rotationAngle How far the contact's ellipse is turned clockwise, in degrees from 0 to 90.
    /// @param force The pressure, from 0 to 1.
    PlatformTouchPoint(uint32 id, State state, const PointD &screenPos, const PointD &pos,
                       const SizeD &radius = {}, float rotationAngle = 0, float force = 0,
                       float twist = 0) noexcept
        : _id(id), _state(state), _screenPos(screenPos), _pos(pos), _radius(radius),
          _rotationAngle(rotationAngle), _twist(twist), _force(force)
    {
    }

    KRYS_NODISCARD uint32 Id() const noexcept
    {
      return _id;
    }

    KRYS_NODISCARD State GetState() const noexcept
    {
      return _state;
    }

    KRYS_NODISCARD PointD ScreenPos() const noexcept
    {
      return _screenPos;
    }

    KRYS_NODISCARD PointD Pos() const noexcept
    {
      return _pos;
    }

    KRYS_NODISCARD SizeD Radius() const noexcept
    {
      return _radius;
    }

    KRYS_NODISCARD float RotationAngle() const noexcept
    {
      return _rotationAngle;
    }

    KRYS_NODISCARD float Twist() const noexcept
    {
      return _twist;
    }

    KRYS_NODISCARD float Force() const noexcept
    {
      return _force;
    }
  };
}
