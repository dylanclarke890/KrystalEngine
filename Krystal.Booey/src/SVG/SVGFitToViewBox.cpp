#include "Krystal.Booey/SVG/SVGFitToViewBox.hpp"
#include "Krystal.Booey/Graphics/Transforms/AffineTransform.hpp"
#include "Krystal.Booey/SVG/Internals/SVGParserUtilities.hpp"
#include "Krystal.Booey/SVG/Properties/SVGPropertyOwnerRegistry.hpp"
#include "Krystal.Booey/SVG/SVGElement.hpp"
#include "Krystal.Booey/SVG/SVGNames.hpp"
#include "Krystal.Booey/SVG/SVGRect.hpp"

namespace krys::boo::svg
{
  SVGFitToViewBox::SVGFitToViewBox(SVGElement *contextElement, SVGPropertyAccess access) noexcept
      : _viewBox(SVGAnimatedRect::Create(contextElement, access)),
        _preserveAspectRatio(SVGAnimatedPreserveAspectRatio::Create(contextElement, access))
  {
    static bool didRegistration = false;
    if (!didRegistration) KRYS_UNLIKELY
    {
      didRegistration = true;
      PropertyRegistry::RegisterProperty<&SVGFitToViewBox::_viewBox>(SVGNames::ViewBoxAttr());
      PropertyRegistry::RegisterProperty<&SVGFitToViewBox::_preserveAspectRatio>(
        SVGNames::PreserveAspectRatioAttr());
    }
  }

  bool SVGFitToViewBox::IsKnownAttribute(const dom::QualifiedName &attributeName) noexcept
  {
    return PropertyRegistry::IsKnownAttribute(attributeName);
  }

  void SVGFitToViewBox::SetViewBox(const FloatRect &viewBox) noexcept
  {
    _viewBox->SetBaseValInternal(viewBox);
    _isViewBoxValid = true;
  }

  void SVGFitToViewBox::ResetViewBox() noexcept
  {
    _viewBox->SetBaseValInternal({});
    _isViewBoxValid = false;
  }

  dom::DOMString SVGFitToViewBox::ViewBoxString() const noexcept
  {
    return SVGPropertyTraits<FloatRect>::ToString(ViewBox());
  }

  void SVGFitToViewBox::Reset() noexcept
  {
    ResetViewBox();
    ResetPreserveAspectRatio();
  }

  bool SVGFitToViewBox::ParseAttribute(const dom::QualifiedName &name,
                                       Maybe<dom::DOMStringView> value) noexcept
  {
    if (name == SVGNames::ViewBoxAttr())
    {
      if (value)
      {
        if (auto result = ParseViewBox(*value))
        {
          SetViewBox(*result);
          return true;
        }
      }
      ResetViewBox();
      return true;
    }

    if (name == SVGNames::PreserveAspectRatioAttr())
    {
      SetPreserveAspectRatio(SVGPreserveAspectRatioValue {value.value_or(dom::DOMStringView {})});
      return true;
    }

    return false;
  }

  Maybe<FloatRect> SVGFitToViewBox::ParseViewBox(dom::DOMStringView value) noexcept
  {
    return ParseViewBox(value, true);
  }

  Maybe<FloatRect> SVGFitToViewBox::ParseViewBox(dom::DOMStringView &buffer, bool validate) noexcept
  {
    SkipOptionalSVGSpaces(buffer);

    auto x = ParseNumber(buffer);
    auto y = ParseNumber(buffer);
    auto width = ParseNumber(buffer);
    auto height = ParseNumber(buffer, SuffixSkippingPolicy::DontSkip);

    if (validate)
    {
      // FIXME: Report the problems below to the document's SVG extensions (reportWarning, reportError), once
      // there's a console.
      if (!x || !y || !width || !height)
      {
        return {};
      }

      // Check that width is positive.
      if (*width < 0.0)
      {
        return {};
      }

      // Check that height is positive.
      if (*height < 0.0)
      {
        return {};
      }

      // Nothing should come after the last, fourth number.
      SkipOptionalSVGSpaces(buffer);
      if (!buffer.empty())
      {
        return {};
      }
    }

    return FloatRect {x.value_or(0), y.value_or(0), width.value_or(0), height.value_or(0)};
  }

  AffineTransform
    SVGFitToViewBox::ViewBoxToViewTransform(const FloatRect &viewBoxRect,
                                            const SVGPreserveAspectRatioValue &preserveAspectRatio,
                                            float viewWidth, float viewHeight) noexcept
  {
    if (!viewBoxRect.Width() || !viewBoxRect.Height() || !viewWidth || !viewHeight)
    {
      return AffineTransform();
    }

    return preserveAspectRatio.GetCTM(viewBoxRect.X(), viewBoxRect.Y(), viewBoxRect.Width(),
                                      viewBoxRect.Height(), viewWidth, viewHeight);
  }
}
