#pragma once

#include "Krystal.Booey/Graphics/Color/ColorInterpolationMethod.hpp"
#include "Krystal.Booey/Graphics/FloatPoint.hpp"
#include "Krystal.Booey/Graphics/GradientColorStops.hpp"
#include "Krystal.Booey/Graphics/GraphicsTypes.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/Variant.hpp"

namespace krys
{
  class TextStream;
}

namespace krys::boo
{
  class FloatRect;

  /// @brief A linear, radial or conic gradient: its geometry, colour stops, interpolation and spread.
  /// FIXME: Drawing (`fill`, the platform shaders and patterns) and the rendering resource observers come
  /// with the graphics backend.
  class Gradient final : public ThreadSafeRefCounted<Gradient>
  {
  public:
    struct LinearData
    {
      FloatPoint Point0;
      FloatPoint Point1;
    };

    struct RadialData
    {
      FloatPoint Point0;
      FloatPoint Point1;
      float StartRadius;
      float EndRadius;
      float AspectRatio; // For elliptical gradient, width / height.
    };

    struct ConicData
    {
      FloatPoint Point0;
      float AngleRadians;
    };

    using Data = Variant<LinearData, RadialData, ConicData>;

  private:
    Data _data;
    ColourInterpolationMethod _colorInterpolationMethod;
    GradientSpreadMethod _spreadMethod;
    GradientColorStops _stops;
    mutable uint32 _cachedHash {0};
    bool _isTransient {true};

  public:
    /// @param isTransient Whether the gradient is drawn only a few times, so a backend needn't cache for it.
    KRYS_NODISCARD static Ref<Gradient> Create(Data &&, ColourInterpolationMethod,
                                               GradientSpreadMethod = GradientSpreadMethod::Pad,
                                               GradientColorStops && = {}, bool isTransient = true) noexcept;
    ~Gradient() noexcept;

    KRYS_NODISCARD const Data &GetData() const noexcept
    {
      return _data;
    }
    KRYS_NODISCARD ColourInterpolationMethod GetColorInterpolationMethod() const noexcept
    {
      return _colorInterpolationMethod;
    }
    KRYS_NODISCARD GradientSpreadMethod SpreadMethod() const noexcept
    {
      return _spreadMethod;
    }
    KRYS_NODISCARD const GradientColorStops &Stops() const noexcept
    {
      return _stops;
    }
    KRYS_NODISCARD bool IsTransient() const noexcept
    {
      return _isTransient;
    }

    void AddColorStop(GradientColorStop &&) noexcept;

    KRYS_NODISCARD bool IsZeroSize() const noexcept;

    void AdjustParametersForTiledDrawing(FloatSize &, FloatRect &, const FloatSize &spacing) noexcept;

    /// @brief A hash of the geometry, interpolation, spread and stops, for caches drawn from the gradient.
    KRYS_NODISCARD uint32 Hash() const noexcept;

  private:
    Gradient(Data &&, ColourInterpolationMethod, GradientSpreadMethod, GradientColorStops &&,
             bool isTransient) noexcept;
  };
}

namespace krys::boo
{
  TextStream &operator<<(TextStream &ts, const Gradient &gradient) noexcept;
}
