#include "Krystal.Booey/DOM/Touch.hpp"
#include "Krystal.Booey/DOM/EventTarget.hpp"
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
