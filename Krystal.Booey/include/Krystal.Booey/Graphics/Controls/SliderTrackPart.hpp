#pragma once

#include "Krystal.Booey/Graphics/Controls/ControlPart.hpp"
#include "Krystal.Booey/Graphics/IntRect.hpp"
#include "Krystal.Core/Types/List.hpp"

namespace krys::boo
{
  /// @brief A slider's track: where its thumb is, and the ticks of its data list's suggestions.
  class SliderTrackPart : public ControlPart
  {
  private:
    IntSize _thumbSize;
    IntRect _trackBounds;
    List<double> _tickRatios;
    double _thumbPosition;

  public:
    KRYS_NODISCARD static Ref<SliderTrackPart> Create(StyleAppearance) noexcept;
    KRYS_NODISCARD static Ref<SliderTrackPart> Create(StyleAppearance, const IntSize &thumbSize,
                                                      const IntRect &trackBounds, List<double> &&tickRatios,
                                                      double thumbPosition) noexcept;

    KRYS_NODISCARD IntSize ThumbSize() const noexcept
    {
      return _thumbSize;
    }

    void SetThumbSize(IntSize thumbSize) noexcept
    {
      _thumbSize = thumbSize;
    }

    KRYS_NODISCARD IntRect TrackBounds() const noexcept
    {
      return _trackBounds;
    }

    void SetTrackBounds(IntRect trackBounds) noexcept
    {
      _trackBounds = trackBounds;
    }

    KRYS_NODISCARD const List<double> &TickRatios() const noexcept
    {
      return _tickRatios;
    }

    void SetTickRatios(List<double> &&tickRatios) noexcept
    {
      _tickRatios = krys::move(tickRatios);
    }

    KRYS_NODISCARD double ThumbPosition() const noexcept
    {
      return _thumbPosition;
    }

    void SetThumbPosition(double thumbPosition) noexcept
    {
      _thumbPosition = thumbPosition;
    }

    void DrawTicks(GraphicsContext &, const FloatRect &, const ControlStyle &) const noexcept;

  private:
    SliderTrackPart(StyleAppearance, const IntSize &thumbSize, const IntRect &trackBounds,
                    List<double> &&tickRatios, double thumbPosition) noexcept;

    KRYS_NODISCARD UniquePtr<PlatformControl> CreatePlatformControl() noexcept override;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::SliderTrackPart)
  KRYS_NODISCARD static bool IsType(const krys::boo::ControlPart &part) noexcept
  {
    return part.GetType() == krys::boo::StyleAppearance::SliderHorizontal
           || part.GetType() == krys::boo::StyleAppearance::SliderVertical;
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
