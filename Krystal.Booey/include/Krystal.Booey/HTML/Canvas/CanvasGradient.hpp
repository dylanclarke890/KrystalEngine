#pragma once

#include "Krystal.Booey/DOM/Types/DOMString.hpp"
#include "Krystal.Booey/DOM/Types/ExceptionOr.hpp"
#include "Krystal.Booey/Graphics/FloatPoint.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo
{
  class Gradient;
}

namespace krys::boo::html
{
  /// @brief A gradient a 2D context fills or strokes with.
  /// @see https://html.spec.whatwg.org/#canvasgradient
  class CanvasGradient : public RefCounted<CanvasGradient>
  {
  private:
    const Ref<Gradient> _gradient;

  public:
    KRYS_NODISCARD static Ref<CanvasGradient> Create(const FloatPoint &p0, const FloatPoint &p1) noexcept;
    KRYS_NODISCARD static Ref<CanvasGradient> Create(const FloatPoint &p0, float r0, const FloatPoint &p1,
                                                     float r1) noexcept;
    KRYS_NODISCARD static Ref<CanvasGradient> Create(const FloatPoint &centerPoint,
                                                     float angleInRadians) noexcept;

    ~CanvasGradient() noexcept;

    KRYS_NODISCARD Gradient &GetGradient() noexcept
    {
      return *_gradient;
    }
    KRYS_NODISCARD const Gradient &GetGradient() const noexcept
    {
      return *_gradient;
    }

    /// @see https://html.spec.whatwg.org/#dom-canvasgradient-addcolorstop
    dom::ExceptionOr<void> AddColorStop(double value, dom::DOMStringView color) noexcept;

  private:
    CanvasGradient(const FloatPoint &p0, const FloatPoint &p1) noexcept;
    CanvasGradient(const FloatPoint &p0, float r0, const FloatPoint &p1, float r1) noexcept;
    CanvasGradient(const FloatPoint &centerPoint, float angleInRadians) noexcept;
  };
}
