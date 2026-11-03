#include "Krystal.Booey/HTML/Canvas/CanvasGradient.hpp"
#include "Krystal.Booey/Graphics/Gradient.hpp"
#include "Krystal.Booey/HTML/Canvas/CanvasStyle.hpp"
#include <cmath>

namespace krys::boo::html
{
  CanvasGradient::CanvasGradient(const FloatPoint &p0, const FloatPoint &p1) noexcept
      : _gradient(Gradient::Create(Gradient::LinearData {p0, p1}, {ColourInterpolationMethod::SRGB {},
                                                                   AlphaPremultiplication::Unpremultiplied}))
  {
  }

  CanvasGradient::CanvasGradient(const FloatPoint &p0, float r0, const FloatPoint &p1, float r1) noexcept
      : _gradient(
          Gradient::Create(Gradient::RadialData {p0, p1, r0, r1, 1},
                           {ColourInterpolationMethod::SRGB {}, AlphaPremultiplication::Unpremultiplied}))
  {
  }

  CanvasGradient::CanvasGradient(const FloatPoint &centerPoint, float angleInRadians) noexcept
      : _gradient(
          Gradient::Create(Gradient::ConicData {centerPoint, angleInRadians},
                           {ColourInterpolationMethod::SRGB {}, AlphaPremultiplication::Unpremultiplied}))
  {
  }

  Ref<CanvasGradient> CanvasGradient::Create(const FloatPoint &p0, const FloatPoint &p1) noexcept
  {
    return AdoptRef(*new CanvasGradient(p0, p1));
  }

  Ref<CanvasGradient> CanvasGradient::Create(const FloatPoint &p0, float r0, const FloatPoint &p1,
                                             float r1) noexcept
  {
    return AdoptRef(*new CanvasGradient(p0, r0, p1, r1));
  }

  Ref<CanvasGradient> CanvasGradient::Create(const FloatPoint &centerPoint, float angleInRadians) noexcept
  {
    return AdoptRef(*new CanvasGradient(centerPoint, angleInRadians));
  }

  CanvasGradient::~CanvasGradient() noexcept = default;

  // https://html.spec.whatwg.org/#dom-canvasgradient-addcolorstop
  dom::ExceptionOr<void> CanvasGradient::AddColorStop(double value, dom::DOMStringView colorString) noexcept
  {
    // (The offset is a double, so a non-finite one is a TypeError.)
    if (!std::isfinite(value))
    {
      return dom::Exception {dom::ExceptionCode::TypeError};
    }

    // 1. If the offset is less than 0 or greater than 1, then throw an "IndexSizeError" DOMException.
    if (value < 0 || value > 1)
    {
      return dom::Exception {dom::ExceptionCode::IndexSizeError};
    }

    // 2. Let parsed color be the result of parsing color.
    // 3. If parsed color is failure, throw a "SyntaxError" DOMException.
    auto color = ParseColor(colorString);
    if (!color.IsValid())
    {
      return dom::Exception {dom::ExceptionCode::SyntaxError};
    }

    // 4. Place a new stop on the gradient, at offset offset relative to the whole gradient, and whose color
    // is
    //    parsed color.
    _gradient->AddColorStop({static_cast<float>(value), krys::move(color)});
    return {};
  }
}
