#pragma once

#include "Krystal.Booey/SVG/Properties/SVGAnimatedProperty.hpp"
#include "Krystal.Booey/SVG/Properties/SVGProperty.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include <utility>

namespace krys::boo::svg
{
  /// @brief An animated list (of lengths, numbers, points or transforms).
  template <typename TList>
  class SVGAnimatedPropertyList : public SVGAnimatedProperty
  {
  protected:
    Ref<TList> _baseVal;
    mutable RefPtr<TList> _animVal;

  public:
    template <typename... TArgs>
    KRYS_NODISCARD static Ref<SVGAnimatedPropertyList> Create(SVGElement *contextElement,
                                                              TArgs &&...arguments) noexcept
    {
      return AdoptRef(*new SVGAnimatedPropertyList(contextElement, std::forward<TArgs>(arguments)...));
    }

    ~SVGAnimatedPropertyList() noexcept override
    {
      _baseVal->Detach();
      if (_animVal)
      {
        _animVal->Detach();
      }
    }

    // Used by the DOM.
    KRYS_NODISCARD const Ref<TList> &BaseVal() const noexcept
    {
      return _baseVal;
    }

    // Used by SVGElement::AttributeChanged().
    KRYS_NODISCARD Ref<TList> &BaseVal() noexcept
    {
      return _baseVal;
    }

    // Used by the DOM.
    KRYS_NODISCARD const RefPtr<TList> &AnimVal() const noexcept
    {
      return const_cast<SVGAnimatedPropertyList *>(this)->EnsureAnimVal();
    }

    // Called by SVGAnimatedPropertyAnimator to pass the animVal to the SVGAnimationFunction::progress.
    KRYS_NODISCARD RefPtr<TList> &AnimVal() noexcept
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
      krys_assert(IsAnimating());
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
    KRYS_NODISCARD const TList &CurrentValue() const noexcept
    {
      krys_assert(!IsAnimating() || _animVal);
      return IsAnimating() ? *_animVal : *_baseVal;
    }

  protected:
    template <typename... TArgs>
    SVGAnimatedPropertyList(SVGElement *contextElement, TArgs &&...arguments) noexcept
        : SVGAnimatedProperty(contextElement),
          _baseVal(TList::Create(this, SVGPropertyAccess::ReadWrite, std::forward<TArgs>(arguments)...))
    {
    }

    RefPtr<TList> &EnsureAnimVal() noexcept
    {
      if (!_animVal)
      {
        _animVal = TList::Create(*_baseVal, SVGPropertyAccess::ReadOnly);
      }
      return _animVal;
    }

    // Called when _baseVal changes or an item in _baseVal changes.
    void CommitPropertyChange(SVGProperty *property) noexcept override
    {
      if (_animVal)
      {
        *_animVal = *_baseVal;
      }
      SVGAnimatedProperty::CommitPropertyChange(property);
    }
  };
}
