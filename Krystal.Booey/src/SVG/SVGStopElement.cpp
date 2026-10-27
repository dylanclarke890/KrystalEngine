#include "Krystal.Booey/SVG/SVGStopElement.hpp"
#include "Krystal.Booey/Rendering/RenderElement.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Rendering/SVG/RenderSVGGradientStop.hpp"
#include "Krystal.Booey/SVG/Properties/SVGPropertyOwnerRegistry.hpp"
#include "Krystal.Booey/SVG/SVGNames.hpp"
#include "Krystal.Core/Text/ASCII.hpp"
#include <charconv>

namespace krys::boo::svg
{
  namespace
  {
    // The number the string is, after any leading whitespace, or 0 if it isn't one.
    KRYS_NODISCARD float ToFloat(dom::DOMStringView string) noexcept
    {
      while (!string.empty() && text::IsASCIIWhitespace(string.front()))
      {
        string.remove_prefix(1);
      }
      if (!string.empty() && string.front() == u8'+')
      {
        string.remove_prefix(1);
      }

      float value = 0;
      auto first = reinterpret_cast<const char *>(string.data());
      auto last = first + string.size();
      auto result = std::from_chars(first, last, value);
      return result.ec == std::errc {} && result.ptr == last ? value : 0;
    }
  }

  SVGStopElement::SVGStopElement(dom::Document &document) noexcept
      : SVGElement(document, CreateUnique<PropertyRegistry>(*this), SVGElementInterface::Stop),
        _offset(SVGAnimatedNumber::Create(this, 0))
  {
    static bool didRegistration = false;
    if (!didRegistration) KRYS_UNLIKELY
    {
      didRegistration = true;
      PropertyRegistry::RegisterProperty<&SVGStopElement::_offset>(SVGNames::OffsetAttr());
    }
  }

  void SVGStopElement::AttributeChanged(const dom::QualifiedName &name, Maybe<dom::DOMStringView> oldValue,
                                        Maybe<dom::DOMStringView> value) noexcept
  {
    if (name == SVGNames::OffsetAttr())
    {
      auto string = value.value_or(dom::DOMStringView {});
      if (string.ends_with(u8'%'))
      {
        _offset->SetBaseValInternal(ToFloat(string.substr(0, string.size() - 1)) / 100.0f);
      }
      else
      {
        _offset->SetBaseValInternal(ToFloat(string));
      }
    }

    SVGElement::AttributeChanged(name, oldValue, value);
  }

  void SVGStopElement::SVGAttributeChanged(const dom::QualifiedName &attributeName) noexcept
  {
    if (PropertyRegistry::IsKnownAttribute(attributeName))
    {
      krys_debug_assert(attributeName == SVGNames::OffsetAttr());
      InstanceInvalidationGuard guard(*this);
      UpdateSVGRendererForElementChange();
      return;
    }

    SVGElement::SVGAttributeChanged(attributeName);
  }

  RenderPtr<RenderElement> SVGStopElement::CreateElementRenderer(RenderStyle &&style) noexcept
  {
    return CreateRenderer<RenderSVGGradientStop>(*this, krys::move(style));
  }

  bool SVGStopElement::RendererIsNeeded(const RenderStyle &) noexcept
  {
    return true;
  }

  Color SVGStopElement::StopColorIncludingOpacity() const noexcept
  {
    // Return initial value 'black' as per web specification below:
    // https://svgwg.org/svg2-draft/pservers.html#StopColorProperties
    if (!Renderer())
    {
      return Color::black;
    }

    auto &style = Renderer()->Style();
    auto stopColor = style.ColorResolvingCurrentColor(style.StopColor());
    return stopColor.ColorWithAlphaMultipliedBy(style.StopOpacity().Value.Value);
  }
}
