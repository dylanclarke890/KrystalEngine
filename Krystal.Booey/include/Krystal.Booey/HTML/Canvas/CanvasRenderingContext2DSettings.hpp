#pragma once

#include "Krystal.Booey/HTML/Canvas/PredefinedColorSpace.hpp"

namespace krys::boo::html
{
  /// @brief What a 2D context is made with.
  /// @see https://html.spec.whatwg.org/#canvasrenderingcontext2dsettings
  struct CanvasRenderingContext2DSettings
  {
    // FIXME: An opaque backing store; the context is always transparent, as without the setting.
    bool Alpha {true};
    bool Desynchronized {false};
    bool WillReadFrequently {false};
    PredefinedColorSpace ColorSpace {PredefinedColorSpace::SRGB};

    /// @see https://html.spec.whatwg.org/#canvascolortype
    enum class ColorType : bool
    {
      Unorm8,
      Float16,
    };
    // FIXME: Half-float backing stores; the context draws in 8 bits a channel whichever is chosen.
    ColorType ColourType {ColorType::Unorm8};
  };
}
