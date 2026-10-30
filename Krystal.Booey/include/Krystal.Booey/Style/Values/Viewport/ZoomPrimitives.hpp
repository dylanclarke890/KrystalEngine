#pragma once

namespace krys::boo::style
{
  // Token passed around to indicate that the evaluation will need zoom passed in the future.
  struct ZoomNeeded
  {
  };

  struct ZoomFactor
  {
    float Value;

    // FIXME: Transitional member to preserve pixel-snapping behavior during evaluation.
    // This allows values like LineWidth to apply floorToDevicePixel() at evaluation time.
    // Should be removed once pixel-snapping is eliminated entirely.
    // https://bugs.webkit.org/show_bug.cgi?id=300658
    float DeviceScaleFactor {1.0f};

    constexpr explicit ZoomFactor(float v, float dsf) noexcept : Value(v), DeviceScaleFactor(dsf)
    {
    }
  };
}
