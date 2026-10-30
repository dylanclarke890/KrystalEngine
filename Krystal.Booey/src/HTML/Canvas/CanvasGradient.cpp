#include "Krystal.Booey/HTML/Canvas/CanvasGradient.hpp"
#include "Krystal.Booey/Graphics/Gradient.hpp"
#include "Krystal.Booey/HTML/Canvas/CanvasStyle.hpp"

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

  dom::ExceptionOr<void> CanvasGradient::AddColorStop(double value, dom::DOMStringView colorString) noexcept
  {
    if (!(value >= 0 && value <= 1))
    {
      return dom::Exception {dom::ExceptionCode::IndexSizeError};
    }

    auto color = ParseColor(colorString);
    if (!color.IsValid())
    {
      return dom::Exception {dom::ExceptionCode::SyntaxError};
    }

    _gradient->AddColorStop({static_cast<float>(value), krys::move(color)});
    return {};
  }
}
