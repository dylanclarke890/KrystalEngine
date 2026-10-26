#pragma once

#include "Krystal.Booey/DOM/Types/ExceptionOr.hpp"
#include "Krystal.Booey/SVG/Properties/SVGAnimatedProperty.hpp"
#include "Krystal.Booey/SVG/Properties/SVGSharedPrimitiveProperty.hpp"

namespace krys::boo::svg
{
  /// @brief An animated boolean, integer, number or string.
  template <typename TProperty>
  class SVGAnimatedPrimitiveProperty : public SVGAnimatedProperty
  {
  protected:
    const Ref<SVGSharedPrimitiveProperty<TProperty>> _baseVal;
    mutable RefPtr<SVGSharedPrimitiveProperty<TProperty>> _animVal;

  public:
    using ValueType = TProperty;

    KRYS_NODISCARD static Ref<SVGAnimatedPrimitiveProperty> Create(SVGElement *contextElement) noexcept
    {
      return AdoptRef(*new SVGAnimatedPrimitiveProperty(contextElement));
    }

    KRYS_NODISCARD static Ref<SVGAnimatedPrimitiveProperty> Create(SVGElement *contextElement,
                                                                   const TProperty &value) noexcept
    {
      return AdoptRef(*new SVGAnimatedPrimitiveProperty(contextElement, value));
    }

    // Used by the DOM.
    dom::ExceptionOr<void> SetBaseVal(const TProperty &baseVal) noexcept
    {
      _baseVal->SetValue(baseVal);
      CommitPropertyChange(nullptr);
      return {};
    }

    // Used by SVGElement::AttributeChanged().
    void SetBaseValInternal(const TProperty &baseVal) noexcept
    {
      _baseVal->SetValue(baseVal);
    }

    KRYS_NODISCARD const TProperty &BaseVal() const noexcept
    {
      return _baseVal->Value();
    }

    // Used by SVGAttributeAnimator::progress.
    void SetAnimVal(const TProperty &animVal) noexcept
    {
      krys_assert(IsAnimating() && _animVal);
      _animVal->SetValue(animVal);
    }

    KRYS_NODISCARD const TProperty &AnimVal() const noexcept
    {
      krys_assert(!IsAnimating() || _animVal);
      return IsAnimating() ? _animVal->Value() : _baseVal->Value();
    }

    KRYS_NODISCARD TProperty &AnimVal() noexcept
    {
      krys_assert(!IsAnimating() || _animVal);
      return IsAnimating() ? _animVal->Value() : _baseVal->Value();
    }

    // Used when committing a change from the SVGAnimatedProperty to the attribute.
    KRYS_NODISCARD dom::DOMString BaseValAsString() const noexcept override
    {
      return _baseVal->ValueAsString();
    }

    // Used to apply the SVGAttributeAnimator change to the target element.
    KRYS_NODISCARD dom::DOMString AnimValAsString() const noexcept override
    {
      krys_assert(IsAnimating() && _animVal);
      return _animVal->ValueAsString();
    }

    // Managing the relationship with the owner.
    void SetDirty() noexcept override
    {
      _baseVal->SetDirty();
    }

    KRYS_NODISCARD bool IsDirty() const noexcept override
    {
      return _baseVal->IsDirty();
    }

    Maybe<dom::DOMString> Synchronize() noexcept override
    {
      return _baseVal->Synchronize();
    }

    // Used by RenderSVGElements.
    KRYS_NODISCARD const TProperty &CurrentValue() const noexcept
    {
      krys_assert(!IsAnimating() || _animVal);
      return IsAnimating() ? _animVal->Value() : _baseVal->Value();
    }

  protected:
    SVGAnimatedPrimitiveProperty(SVGElement *contextElement) noexcept
        : SVGAnimatedProperty(contextElement), _baseVal(SVGSharedPrimitiveProperty<TProperty>::Create())
    {
    }

    SVGAnimatedPrimitiveProperty(SVGElement *contextElement, const TProperty &value) noexcept
        : SVGAnimatedProperty(contextElement), _baseVal(SVGSharedPrimitiveProperty<TProperty>::Create(value))
    {
    }

    RefPtr<SVGSharedPrimitiveProperty<TProperty>> &EnsureAnimVal() noexcept
    {
      if (!_animVal)
      {
        _animVal = SVGSharedPrimitiveProperty<TProperty>::Create(_baseVal->Value());
      }
      return _animVal;
    }
  };
}
