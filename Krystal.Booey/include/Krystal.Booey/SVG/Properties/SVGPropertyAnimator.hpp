#pragma once

#include "Krystal.Booey/CSS/Parser/KeywordLookup.hpp"
#include "Krystal.Booey/CSS/Values/SerialisationContext.hpp"
#include "Krystal.Booey/Style/Extractor.hpp"
#include "Krystal.Booey/SVG/Properties/SVGAttributeAnimator.hpp"
#include "Krystal.Booey/SVG/SVGElement.hpp"
#include "Krystal.Core/TypeCast.hpp"
#include <utility>

namespace krys::boo::svg
{
  /// @brief Animates a presentation attribute's animated style property with @p TAnimationFunction.
  template <typename TAnimationFunction>
  class SVGPropertyAnimator : public SVGAttributeAnimator
  {
  protected:
    TAnimationFunction _function;

  public:
    KRYS_NODISCARD bool IsDiscrete() const noexcept override
    {
      return _function.IsDiscrete();
    }

    void SetFromAndToValues(SVGElement &targetElement, dom::DOMStringView from,
                            dom::DOMStringView to) noexcept override
    {
      _function.SetFromAndToValues(targetElement, AdjustForInheritance(targetElement, from),
                                   AdjustForInheritance(targetElement, to));
    }

    void SetFromAndByValues(SVGElement &targetElement, dom::DOMStringView from,
                            dom::DOMStringView by) noexcept override
    {
      _function.SetFromAndByValues(targetElement, from, by);
    }

    void SetToAtEndOfDurationValue(SVGElement &targetElement,
                                   dom::DOMStringView toAtEndOfDuration) noexcept override
    {
      _function.SetToAtEndOfDurationValue(targetElement, toAtEndOfDuration);
    }

  protected:
    template <typename... TArgs>
    SVGPropertyAnimator(const dom::QualifiedName &attributeName, TArgs &&...arguments) noexcept
        : SVGAttributeAnimator(attributeName), _function(std::forward<TArgs>(arguments)...)
    {
    }

    void Stop(SVGElement &targetElement) noexcept override
    {
      RemoveAnimatedStyleProperty(targetElement);
    }

    KRYS_NODISCARD Maybe<float> CalculateDistance(SVGElement &targetElement, dom::DOMStringView from,
                                                  dom::DOMStringView to) const noexcept override
    {
      return _function.CalculateDistance(targetElement, from, to);
    }

    KRYS_NODISCARD dom::DOMString AdjustForInheritance(SVGElement &targetElement,
                                                       dom::DOMStringView value) const noexcept
    {
      return value == u8"inherit" ? ComputeInheritedCSSPropertyValue(targetElement) : dom::DOMString(value);
    }

    KRYS_NODISCARD dom::DOMString ComputeCSSPropertyValue(SVGElement &targetElement,
                                                          css::PropertyId id) const noexcept
    {
      Ref protector = ShareRef(targetElement);

      // Don't include any properties resulting from CSS Transitions/Animations or SMIL animations, as we want
      // to retrieve the "base value".
      targetElement.SetUseOverrideComputedStyle(true);
      auto serialization =
        style::Extractor(&targetElement).PropertyValueSerialisation(id, css::DefaultSerialisationContext());
      targetElement.SetUseOverrideComputedStyle(false);

      return dom::DOMString(serialization);
    }

    KRYS_NODISCARD dom::DOMString ComputeInheritedCSSPropertyValue(SVGElement &targetElement) const noexcept
    {
      RefPtr svgParent = ShareRefPtr(DynamicDowncast<SVGElement>(targetElement.ParentElement()));
      if (!svgParent)
      {
        return {};
      }
      return ComputeCSSPropertyValue(*svgParent, css::PropertyIdFromName(_attributeName.LocalName().View()));
    }
  };
}
