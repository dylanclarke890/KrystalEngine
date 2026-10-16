#pragma once

#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/Style/PseudoElementIdentifier.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"

namespace krys::boo::style
{
  /// @brief An element, or one of its pseudo-elements: something that is styled.
  // FIXME: The animation half (keyframe effects, CSS animations and transitions, scroll and view timelines,
  // the last style change event style) comes with animations; the renderer with the render tree.
  struct Styleable
  {
    dom::Element &Element;
    Maybe<style::PseudoElementIdentifier> PseudoElementIdentifier;

    Styleable(dom::Element &element,
              const Maybe<style::PseudoElementIdentifier> &pseudoElementIdentifier) noexcept
        : Element(element), PseudoElementIdentifier(pseudoElementIdentifier)
    {
    }

    KRYS_NODISCARD static const Styleable FromElement(dom::Element &element) noexcept
    {
      return Styleable(element, null);
    }

    KRYS_NODISCARD bool operator==(const Styleable &other) const noexcept
    {
      return &Element == &other.Element && PseudoElementIdentifier == other.PseudoElementIdentifier;
    }

    KRYS_NODISCARD Ref<dom::Element> ProtectedElement() const noexcept
    {
      return ShareRef(Element);
    }
  };

  /// @brief A weak reference to a `Styleable`.
  class WeakStyleable
  {
  private:
    WeakPtr<dom::Element> _element;
    Maybe<style::PseudoElementIdentifier> _pseudoElementIdentifier;

  public:
    WeakStyleable() noexcept = default;

    WeakStyleable(const Styleable &styleable) noexcept
        : _element(CreateWeakPtr(&styleable.Element)),
          _pseudoElementIdentifier(styleable.PseudoElementIdentifier)
    {
    }

    WeakStyleable &operator=(const Styleable &styleable) noexcept
    {
      _element = CreateWeakPtr(&styleable.Element);
      _pseudoElementIdentifier = styleable.PseudoElementIdentifier;
      return *this;
    }

    KRYS_NODISCARD explicit operator bool() const noexcept
    {
      return !!_element;
    }

    KRYS_NODISCARD bool operator==(const WeakStyleable &other) const noexcept = default;

    KRYS_NODISCARD Maybe<Styleable> GetStyleable() const noexcept
    {
      if (!_element)
      {
        return null;
      }
      return Styleable(*_element, _pseudoElementIdentifier);
    }

    KRYS_NODISCARD const WeakPtr<dom::Element> &GetElement() const noexcept
    {
      return _element;
    }

    KRYS_NODISCARD const Maybe<style::PseudoElementIdentifier> &GetPseudoElementIdentifier() const noexcept
    {
      return _pseudoElementIdentifier;
    }
  };
}
