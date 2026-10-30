#include "Krystal.Booey/Graphics/Controls/ControlPart.hpp"
#include "Krystal.Booey/Graphics/FloatRoundedRect.hpp"
#include "Krystal.Booey/Graphics/GraphicsContext.hpp"

namespace krys::boo
{
  ControlPart::ControlPart(StyleAppearance type) noexcept : _type(type)
  {
  }

  ControlFactory &ControlPart::GetControlFactory() const noexcept
  {
    return _overrideControlFactory ? *_overrideControlFactory : ControlFactory::Singleton();
  }

  PlatformControl *ControlPart::GetPlatformControl() const noexcept
  {
    if (!_platformControl)
    {
      _platformControl = const_cast<ControlPart &>(*this).CreatePlatformControl();
    }
    return _platformControl.get();
  }

  FloatSize ControlPart::SizeForBounds(const FloatRect &bounds, const ControlStyle &style) noexcept
  {
    auto *platformControl = GetPlatformControl();
    if (!platformControl)
    {
      return bounds.Size();
    }

    platformControl->UpdateCellStates(bounds, style);
    return platformControl->SizeForBounds(bounds, style);
  }

  FloatRect ControlPart::RectForBounds(const FloatRect &bounds, const ControlStyle &style) noexcept
  {
    auto *platformControl = GetPlatformControl();
    if (!platformControl)
    {
      return bounds;
    }

    platformControl->UpdateCellStates(bounds, style);
    return platformControl->RectForBounds(bounds, style);
  }

  void ControlPart::Draw(GraphicsContext &context, const FloatRoundedRect &borderRect,
                         float deviceScaleFactor, const ControlStyle &style) const noexcept
  {
    auto *platformControl = GetPlatformControl();
    if (!platformControl)
    {
      return;
    }

    // It's important to get the clip from the context, because it may be significantly
    // smaller than the layer bounds (e.g. tiled layers)
    platformControl->SetFocusRingClipRect(FloatRect(context.ClipBounds()));

    platformControl->UpdateCellStates(borderRect.Rect(), style);
    platformControl->Draw(context, borderRect, deviceScaleFactor, style);

    platformControl->SetFocusRingClipRect({});
  }
}
