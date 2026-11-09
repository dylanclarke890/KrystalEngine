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

#include "Krystal.Booey/DOM/Events/Touch.hpp"
#include "Krystal.Booey/DOM/Events/EventTarget.hpp"
#include "Krystal.Booey/Page/LocalFrame.hpp"
#include "Krystal.Booey/Page/LocalFrameView.hpp"

namespace krys::boo::dom
{
  namespace
  {
    KRYS_NODISCARD IntPoint ContentsOffset(LocalFrame *frame) noexcept
    {
      if (!frame)
      {
        return IntPoint();
      }
      auto *frameView = frame->View();
      if (!frameView)
      {
        return IntPoint();
      }
      float scale = 1.0f / frame->PageZoomFactor();
      return frameView->GetScrollPosition().Scaled(scale);
    }

    KRYS_NODISCARD DoublePoint ScaledLocation(LocalFrame *frame, const DoublePoint &pagePosition) noexcept
    {
      if (!frame || !frame->View())
      {
        return pagePosition;
      }
      float scaleFactor = frame->PageZoomFactor() * frame->View()->FrameScaleFactor();
      return pagePosition.Scaled(scaleFactor);
    }
  }

  Ref<Touch> Touch::Create(LocalFrame *frame, EventTarget *target, int32 identifier,
                           const DoublePoint &screenPos, const DoublePoint &pagePos, const DoubleSize &radius,
                           float rotationAngle, float force, double twist) noexcept
  {
    return AdoptRef(
      *new Touch(frame, target, identifier, screenPos, pagePos, radius, rotationAngle, twist, force));
  }

  Touch::Touch(LocalFrame *frame, EventTarget *target, int32 identifier, const DoublePoint &screenPosition,
               const DoublePoint &pagePosition, const DoubleSize &radius, float rotationAngle, double twist,
               float force) noexcept
      : _target(ShareRefPtr(target)), _identifier(identifier),
        _clientPosition(pagePosition - DoubleSize(ContentsOffset(frame).X(), ContentsOffset(frame).Y())),
        _screenPosition(screenPosition), _pagePosition(pagePosition), _radius(radius),
        _rotationAngle(rotationAngle), _twist(twist), _force(force),
        _absoluteLocation(ScaledLocation(frame, pagePosition))
  {
  }

  Touch::Touch(EventTarget *target, int32 identifier, const DoublePoint &clientPosition,
               const DoublePoint &screenPosition, const DoublePoint &pagePosition, const DoubleSize &radius,
               float rotationAngle, double twist, float force, const DoublePoint &absoluteLocation) noexcept
      : _target(ShareRefPtr(target)), _identifier(identifier), _clientPosition(clientPosition),
        _screenPosition(screenPosition), _pagePosition(pagePosition), _radius(radius),
        _rotationAngle(rotationAngle), _twist(twist), _force(force), _absoluteLocation(absoluteLocation)
  {
  }

  Touch::~Touch() noexcept = default;

  Ref<Touch> Touch::CloneWithNewTarget(EventTarget *eventTarget) const noexcept
  {
    return AdoptRef(*new Touch(eventTarget, _identifier, _clientPosition, _screenPosition, _pagePosition,
                               _radius, _rotationAngle, _twist, _force, _absoluteLocation));
  }
}
