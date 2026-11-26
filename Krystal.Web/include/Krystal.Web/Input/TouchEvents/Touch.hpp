/*
 * Copyright 2008, The Android Open Source Project
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Geometry/Point.hpp"
#include "Krystal.Core/Geometry/Size.hpp"
#include "Krystal.Core/Memory/RefCounted.hpp"
#include "Krystal.Core/Memory/RefPtr.hpp"

namespace krys::web
{
  class EventTarget;
  class Frame;

  /// @brief A touch on the screen, as a touch event reports it: where it is, the element it began on and the
  /// size and pressure of its contact.
  /// @see https://w3c.github.io/touch-events/#touch-interface
  class Touch : public RefCounted<Touch>
  {
  private:
    RefPtr<EventTarget> _target;
    int32 _identifier {0};
    // Position relative to the viewport in CSS px.
    PointD _clientPosition;
    // Position relative to the screen in DIPs.
    PointD _screenPosition;
    // Position relative to the page in CSS px.
    PointD _pagePosition;
    // Radius in CSS px.
    SizeD _radius;
    float _rotationAngle {0};
    // Twist is stored so that it can be exposed for pointer events.
    double _twist {0};
    float _force {0};
    PointD _absoluteLocation;

  public:
    KRYS_NODISCARD static Ref<Touch> Create(Frame *frame, EventTarget *target, int32 identifier,
                                            const PointD &screenPos, const PointD &pagePos,
                                            const SizeD &radius, float rotationAngle, float force,
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

    KRYS_NODISCARD const PointD &AbsoluteLocation() const noexcept
    {
      return _absoluteLocation;
    }

    KRYS_NODISCARD Ref<Touch> CloneWithNewTarget(EventTarget *eventTarget) const noexcept;

  private:
    Touch(Frame *frame, EventTarget *target, int32 identifier, const PointD &screenPosition,
          const PointD &pagePosition, const SizeD &radius, float rotationAngle, double twist,
          float force) noexcept;

    Touch(EventTarget *target, int32 identifier, const PointD &clientPosition, const PointD &screenPosition,
          const PointD &pagePosition, const SizeD &radius, float rotationAngle, double twist, float force,
          const PointD &absoluteLocation) noexcept;
  };
}
