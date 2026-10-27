#pragma once

#include "Krystal.Booey/DOM/Types/DOMString.hpp"
#include "Krystal.Booey/SVG/Properties/SVGAttributeAnimator.hpp"
#include "Krystal.Booey/SVG/Properties/SVGPropertyOwner.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"
#include <algorithm>

namespace krys::boo::svg
{
  class SVGElement;

  /// @brief An element's animatable attribute, with its base value and the value an animation gives it.
  /// @see https://svgwg.org/svg2-draft/types.html#InterfaceSVGAnimatedLength
  class SVGAnimatedProperty : public RefCounted<SVGAnimatedProperty>, public SVGPropertyOwner
  {
  protected:
    WeakPtr<SVGElement> _contextElement;
    List<WeakPtr<SVGAttributeAnimator>> _animators;

  public:
    ~SVGAnimatedProperty() noexcept override = default;

    // Manage the relationship with the owner.
    KRYS_NODISCARD bool IsAttached() const noexcept
    {
      return !!_contextElement;
    }

    void Detach() noexcept
    {
      _contextElement = nullptr;
    }

    KRYS_NODISCARD SVGElement *ContextElement() const noexcept;

    KRYS_NODISCARD virtual dom::DOMString BaseValAsString() const noexcept
    {
      return {};
    }

    KRYS_NODISCARD virtual dom::DOMString AnimValAsString() const noexcept
    {
      return {};
    }

    // Control the synchronization between the attribute and its reflection in baseVal.
    KRYS_NODISCARD virtual bool IsDirty() const noexcept
    {
      return false;
    }

    virtual void SetDirty() noexcept
    {
    }

    virtual Maybe<dom::DOMString> Synchronize() noexcept
    {
      return {};
    }

    // Control the animation life cycle.
    KRYS_NODISCARD bool IsAnimating() const noexcept
    {
      return std::ranges::any_of(_animators, [](const auto &animator) { return !!animator; });
    }

    virtual void StartAnimation(SVGAttributeAnimator &animator) noexcept
    {
      std::erase_if(_animators, [](const auto &weakAnimator) { return !weakAnimator; });
      if (std::ranges::none_of(_animators,
                               [&](const auto &weakAnimator) { return weakAnimator.get() == &animator; }))
      {
        _animators.push_back(CreateWeakPtr(&animator));
      }
    }

    virtual void StopAnimation(SVGAttributeAnimator &animator) noexcept
    {
      std::erase_if(_animators, [&](const auto &weakAnimator)
                    { return !weakAnimator || weakAnimator.get() == &animator; });
    }

    // Attach/Detach the animVal of the target element's property by the instance element's property.
    virtual void InstanceStartAnimation(SVGAttributeAnimator &animator, SVGAnimatedProperty &) noexcept
    {
      StartAnimation(animator);
    }

    virtual void InstanceStopAnimation(SVGAttributeAnimator &animator) noexcept
    {
      StopAnimation(animator);
    }

  protected:
    explicit SVGAnimatedProperty(SVGElement *) noexcept;

    KRYS_NODISCARD SVGPropertyOwner *Owner() const noexcept override;
    void CommitPropertyChange(SVGProperty *) noexcept override;
  };
}
