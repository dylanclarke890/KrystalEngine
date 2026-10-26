#pragma once

#include "Krystal.Booey/SVG/Properties/SVGAnimatedProperty.hpp"
#include "Krystal.Booey/SVG/Properties/SVGProperty.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include <utility>

namespace krys::boo::svg
{
  /// @brief An animated value property (an angle, length, rectangle or `preserveAspectRatio`), whose base and
  /// animated values are `TProperty`s the DOM can hold.
  template <typename TProperty>
  class SVGAnimatedValueProperty : public SVGAnimatedProperty
  {
  protected:
    Ref<TProperty> _baseVal;
    mutable RefPtr<TProperty> _animVal;

  public:
    using ValueType = typename TProperty::ValueType;

    template <typename... TArgs>
    KRYS_NODISCARD static Ref<SVGAnimatedValueProperty> Create(SVGElement *contextElement,
                                                               TArgs &&...arguments) noexcept
    {
      return AdoptRef(*new SVGAnimatedValueProperty(contextElement, std::forward<TArgs>(arguments)...));
    }

    ~SVGAnimatedValueProperty() noexcept override
    {
      _baseVal->Detach();
      if (_animVal)
      {
        _animVal->Detach();
      }
    }

    // Used by SVGElement::AttributeChanged().
    void SetBaseValInternal(const ValueType &baseVal) noexcept
    {
      _baseVal->SetValue(baseVal);
      if (_animVal)
      {
        _animVal->SetValue(baseVal);
      }
    }

    // Used by the DOM.
    KRYS_NODISCARD const Ref<TProperty> &BaseVal() const noexcept
    {
      return _baseVal;
    }

    KRYS_NODISCARD Ref<TProperty> &BaseVal() noexcept
    {
      return _baseVal;
    }

    // Used by SVGAnimator::progress.
    void SetAnimVal(const ValueType &animVal) noexcept
    {
      krys_assert(IsAnimating() && _animVal);
      _animVal->SetValue(animVal);
    }

    // Used by the DOM.
    KRYS_NODISCARD const RefPtr<TProperty> &AnimVal() const noexcept
    {
      return const_cast<SVGAnimatedValueProperty *>(this)->EnsureAnimVal();
    }

    // Called by SVGAnimatedPropertyAnimator to pass the animVal to the SVGAnimationFunction::progress.
    KRYS_NODISCARD RefPtr<TProperty> &AnimVal() noexcept
    {
      return EnsureAnimVal();
    }

    // Used when committing a change from the SVGAnimatedProperty to the attribute.
    KRYS_NODISCARD dom::DOMString BaseValAsString() const noexcept override
    {
      return _baseVal->ValueAsString();
    }

    // Used to apply the SVGAnimator change to the target element.
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
    KRYS_NODISCARD const ValueType &CurrentValue() const noexcept
    {
      krys_assert(!IsAnimating() || _animVal);
      return (IsAnimating() ? *_animVal : *_baseVal).Value();
    }

  protected:
    // The packed arguments are used in TProperty creation, for example passing
    // SVGLengthMode to SVGLength.
    template <typename... TArgs>
    SVGAnimatedValueProperty(SVGElement *contextElement, TArgs &&...arguments) noexcept
        : SVGAnimatedProperty(contextElement),
          _baseVal(TProperty::Create(this, SVGPropertyAccess::ReadWrite,
                                     ValueType(std::forward<TArgs>(arguments)...)))
    {
    }

    template <typename... TArgs>
    SVGAnimatedValueProperty(SVGElement *contextElement, SVGPropertyAccess access,
                             TArgs &&...arguments) noexcept
        : SVGAnimatedProperty(contextElement),
          _baseVal(TProperty::Create(this, access, ValueType(std::forward<TArgs>(arguments)...)))
    {
    }

    RefPtr<TProperty> &EnsureAnimVal() noexcept
    {
      if (!_animVal)
      {
        _animVal = TProperty::Create(this, SVGPropertyAccess::ReadOnly, _baseVal->Value());
      }
      return _animVal;
    }

    // Called when _baseVal changes.
    void CommitPropertyChange(SVGProperty *property) noexcept override
    {
      if (_animVal)
      {
        _animVal->SetValue(_baseVal->Value());
      }
      SVGAnimatedProperty::CommitPropertyChange(property);
    }
  };
}
