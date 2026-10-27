#pragma once

#include "Krystal.Booey/DOM/Types/ExceptionOr.hpp"
#include "Krystal.Booey/SVG/Properties/SVGAnimatedProperty.hpp"
#include "Krystal.Booey/SVG/Properties/SVGDecoratedProperty.hpp"
#include "Krystal.Booey/SVG/Properties/SVGProperty.hpp"

namespace krys::boo::svg
{
  /// @brief An animated decorated property: an enumeration the DOM sees as an integer.
  template <template <typename, typename> class TDecoratedProperty, typename TDecoration>
  class SVGAnimatedDecoratedProperty : public SVGAnimatedProperty
  {
  protected:
    const Ref<SVGDecoratedProperty<TDecoration>> _baseVal;
    RefPtr<SVGDecoratedProperty<TDecoration>> _animVal;
    SVGPropertyState _state {SVGPropertyState::Clean};

  public:
    template <typename TProperty, typename TAnimatedProperty = SVGAnimatedDecoratedProperty>
    KRYS_NODISCARD static Ref<TAnimatedProperty> Create(SVGElement *contextElement) noexcept
    {
      return AdoptRef(
        *new TAnimatedProperty(contextElement, AdoptRef(*new TDecoratedProperty<TDecoration, TProperty>())));
    }

    template <typename TProperty, typename TAnimatedProperty = SVGAnimatedDecoratedProperty>
    KRYS_NODISCARD static Ref<TAnimatedProperty> Create(SVGElement *contextElement,
                                                        const TProperty &value) noexcept
    {
      return AdoptRef(
        *new TAnimatedProperty(contextElement, TDecoratedProperty<TDecoration, TProperty>::Create(value)));
    }

    SVGAnimatedDecoratedProperty(SVGElement *contextElement,
                                 Ref<SVGDecoratedProperty<TDecoration>> &&baseVal) noexcept
        : SVGAnimatedProperty(contextElement), _baseVal(krys::move(baseVal))
    {
    }

    // Used by the DOM.
    dom::ExceptionOr<void> SetBaseVal(const TDecoration &baseVal) noexcept
    {
      if (!_baseVal->SetValue(baseVal))
      {
        return dom::Exception {dom::ExceptionCode::TypeError};
      }
      CommitPropertyChange(nullptr);
      return {};
    }

    // Used by SVGElement::AttributeChanged().
    template <typename TProperty>
    void SetBaseValInternal(const TProperty &baseVal) noexcept
    {
      _baseVal->SetValueInternal(static_cast<TDecoration>(baseVal));
      if (_animVal)
      {
        _animVal->SetValueInternal(static_cast<TDecoration>(baseVal));
      }
    }

    KRYS_NODISCARD TDecoration BaseVal() const noexcept
    {
      return _baseVal->Value();
    }

    // Used by SVGAnimator::progress.
    template <typename TProperty>
    void SetAnimVal(const TProperty &animVal) noexcept
    {
      krys_assert(IsAnimating() && _animVal);
      _animVal->SetValueInternal(static_cast<TDecoration>(animVal));
    }

    template <typename TProperty = TDecoration>
    KRYS_NODISCARD TProperty AnimVal() const noexcept
    {
      krys_assert(!IsAnimating() || _animVal);
      return static_cast<TProperty>((IsAnimating() ? *_animVal : *_baseVal).Value());
    }

    // Used when committing a change from the SVGAnimatedProperty to the attribute.
    KRYS_NODISCARD dom::DOMString BaseValAsString() const noexcept override
    {
      return _baseVal->ValueAsString();
    }

    // Used to apply the SVGAnimator change to the target element.
    KRYS_NODISCARD dom::DOMString AnimValAsString() const noexcept override
    {
      krys_assert(IsAnimating() && !!_animVal);
      return _animVal->ValueAsString();
    }

    // Managing the relationship with the owner.
    void SetDirty() noexcept override
    {
      _state = SVGPropertyState::Dirty;
    }

    KRYS_NODISCARD bool IsDirty() const noexcept override
    {
      return _state == SVGPropertyState::Dirty;
    }

    Maybe<dom::DOMString> Synchronize() noexcept override
    {
      if (_state == SVGPropertyState::Clean)
      {
        return {};
      }
      _state = SVGPropertyState::Clean;
      return BaseValAsString();
    }

    // Used by RenderSVGElements.
    template <typename TProperty>
    KRYS_NODISCARD TProperty CurrentValue() const noexcept
    {
      krys_assert(!IsAnimating() || _animVal);
      return static_cast<TProperty>((IsAnimating() ? *_animVal : *_baseVal).ValueInternal());
    }

    // Controlling the animation.
    void StartAnimation(SVGAttributeAnimator &animator) noexcept override
    {
      if (_animVal)
      {
        _animVal->SetValue(_baseVal->Value());
      }
      else
      {
        _animVal = _baseVal->Clone();
      }
      SVGAnimatedProperty::StartAnimation(animator);
    }

    void StopAnimation(SVGAttributeAnimator &animator) noexcept override
    {
      SVGAnimatedProperty::StopAnimation(animator);
      if (!IsAnimating())
      {
        _animVal = nullptr;
      }
      else if (_animVal)
      {
        _animVal->SetValue(_baseVal->Value());
      }
    }

    // Controlling the instance animation.
    void InstanceStartAnimation(SVGAttributeAnimator &animator,
                                SVGAnimatedProperty &animated) noexcept override
    {
      if (!IsAnimating())
      {
        _animVal = static_cast<SVGAnimatedDecoratedProperty &>(animated)._animVal;
      }
      SVGAnimatedProperty::InstanceStartAnimation(animator, animated);
    }

    void InstanceStopAnimation(SVGAttributeAnimator &animator) noexcept override
    {
      SVGAnimatedProperty::InstanceStopAnimation(animator);
      if (!IsAnimating())
      {
        _animVal = nullptr;
      }
    }
  };
}
