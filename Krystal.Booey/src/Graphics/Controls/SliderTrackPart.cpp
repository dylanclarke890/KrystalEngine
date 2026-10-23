#include "Krystal.Booey/Graphics/Controls/SliderTrackPart.hpp"
#include "Krystal.Booey/Graphics/GraphicsContext.hpp"
#include "Krystal.Booey/Graphics/GraphicsContextStateSaver.hpp"
#include <cmath>

namespace krys::boo
{
  Ref<SliderTrackPart> SliderTrackPart::Create(StyleAppearance type) noexcept
  {
    return AdoptRef(*new SliderTrackPart(type, {}, {}, {}, 0));
  }

  Ref<SliderTrackPart> SliderTrackPart::Create(StyleAppearance type, const IntSize &thumbSize,
                                               const IntRect &trackBounds, List<double> &&tickRatios,
                                               double thumbPosition) noexcept
  {
    return AdoptRef(
      *new SliderTrackPart(type, thumbSize, trackBounds, krys::move(tickRatios), thumbPosition));
  }

  SliderTrackPart::SliderTrackPart(StyleAppearance type, const IntSize &thumbSize, const IntRect &trackBounds,
                                   List<double> &&tickRatios, double thumbPosition) noexcept
      : ControlPart(type), _thumbSize(thumbSize), _trackBounds(trackBounds),
        _tickRatios(krys::move(tickRatios)), _thumbPosition(thumbPosition)
  {
    krys_debug_assert(type == StyleAppearance::SliderHorizontal || type == StyleAppearance::SliderVertical);
  }

  void SliderTrackPart::DrawTicks(GraphicsContext &context, const FloatRect &rect,
                                  const ControlStyle &style) const noexcept
  {
    if (_tickRatios.empty())
    {
      return;
    }

    constexpr FloatSize sliderTickSize = {1, 3};
    constexpr int32 sliderTickOffsetFromTrackCenter = -9;

    bool isHorizontal = GetType() == StyleAppearance::SliderHorizontal;

    auto trackBounds = _trackBounds;
    trackBounds.MoveBy(IntPoint(rect.Location()));

    auto tickSize = isHorizontal ? sliderTickSize : sliderTickSize.TransposedSize();
    tickSize.Scale(style.ZoomFactor);

    FloatPoint tickLocation;
    float tickRegionMargin = 0;
    float tickRegionWidth = 0;

    float offsetFromTrackCenter = sliderTickOffsetFromTrackCenter * style.ZoomFactor;

    if (isHorizontal)
    {
      tickLocation = {0, rect.Center().Y() + offsetFromTrackCenter};
      tickRegionMargin = trackBounds.X() + (_thumbSize.Width() - tickSize.Width()) / 2.0f;
      tickRegionWidth = static_cast<float>(trackBounds.Width() - _thumbSize.Width());
    }
    else
    {
      tickLocation = {rect.Center().X() + offsetFromTrackCenter, 0};
      tickRegionMargin = trackBounds.Y() + (_thumbSize.Height() - tickSize.Height()) / 2.0f;
      tickRegionWidth = static_cast<float>(trackBounds.Height() - _thumbSize.Height());
    }

    auto tickRect = FloatRect {tickLocation, tickSize};

    GraphicsContextStateSaver stateSaver(context);
    context.SetFillColor(style.TextColor);

    bool isVerticalWritingMode = style.States.Contains(ControlStyle::State::VerticalWritingMode);
    bool isInlineFlippedWritingMode = style.States.Contains(ControlStyle::State::InlineFlippedWritingMode);
    bool isInlineFlipped = (!isHorizontal && !isVerticalWritingMode) || isInlineFlippedWritingMode;

    for (auto tickRatio : _tickRatios)
    {
      double value = isInlineFlipped ? 1.0 - tickRatio : tickRatio;
      double tickPosition = std::round(tickRegionMargin + tickRegionWidth * value);
      if (isHorizontal)
      {
        tickRect.SetX(static_cast<float>(tickPosition));
      }
      else
      {
        tickRect.SetY(static_cast<float>(tickPosition));
      }
      context.FillRect(tickRect);
    }
  }

  UniquePtr<PlatformControl> SliderTrackPart::CreatePlatformControl() noexcept
  {
    return GetControlFactory().CreatePlatformSliderTrack(*this);
  }
}
