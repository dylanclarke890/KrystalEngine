#pragma once

#include "Krystal.Booey/CSS/Properties/PropertyId.hpp"
#include "Krystal.Booey/DOM/Types/DOMString.hpp"
#include "Krystal.Booey/DOM/Types/QualifiedName.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/CanMakeWeakPtr.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Types/Maybe.hpp"

namespace krys::boo::svg
{
  class SVGElement;

  /// @see https://svgwg.org/specs/animations/#AnimationModes
  enum class AnimationMode : uint8
  {
    None,
    FromTo,
    FromBy,
    To,
    By,
    Values,
    Path
  };

  /// @see https://svgwg.org/specs/animations/#CalcModeAttribute
  enum class CalcMode : uint8
  {
    Discrete,
    Linear,
    Paced,
    Spline
  };

  /// @brief Animates one attribute of a target element for an animation element: an animated property's
  /// animated value, or a presentation attribute's animated style property.
  class SVGAttributeAnimator : public RefCounted<SVGAttributeAnimator>,
                               public CanMakeWeakPtr<SVGAttributeAnimator>
  {
  protected:
    dom::QualifiedName _attributeName;

  public:
    explicit SVGAttributeAnimator(const dom::QualifiedName &attributeName) noexcept
        : _attributeName(attributeName)
    {
    }

    virtual ~SVGAttributeAnimator() noexcept = default;

    KRYS_NODISCARD virtual bool IsDiscrete() const noexcept
    {
      return false;
    }

    virtual void SetFromAndToValues(SVGElement &, dom::DOMStringView, dom::DOMStringView) noexcept
    {
    }

    virtual void SetFromAndByValues(SVGElement &, dom::DOMStringView, dom::DOMStringView) noexcept
    {
    }

    virtual void SetToAtEndOfDurationValue(SVGElement &, dom::DOMStringView) noexcept
    {
    }

    virtual void Start(SVGElement &) noexcept = 0;
    virtual void Animate(SVGElement &, float progress, uint32 repeatCount) noexcept = 0;
    virtual void Apply(SVGElement &) noexcept = 0;
    virtual void Stop(SVGElement &targetElement) noexcept = 0;

    KRYS_NODISCARD virtual Maybe<float> CalculateDistance(SVGElement &, dom::DOMStringView,
                                                          dom::DOMStringView) const noexcept
    {
      return {};
    }

  protected:
    KRYS_NODISCARD bool IsAnimatedStylePropertyAnimator(const SVGElement &) const noexcept;

    static void InvalidateStyle(SVGElement &) noexcept;
    static void ApplyAnimatedStylePropertyChange(SVGElement &, css::PropertyId,
                                                 dom::DOMStringView value) noexcept;
    static void RemoveAnimatedStyleProperty(SVGElement &, css::PropertyId) noexcept;
    static void ApplyAnimatedPropertyChange(SVGElement &, const dom::QualifiedName &) noexcept;

    void ApplyAnimatedStylePropertyChange(SVGElement &, dom::DOMStringView value) noexcept;
    void RemoveAnimatedStyleProperty(SVGElement &) noexcept;
    void ApplyAnimatedPropertyChange(SVGElement &) noexcept;
  };
}
