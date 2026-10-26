#pragma once

#include "Krystal.Booey/DOM/Types/DOMString.hpp"
#include "Krystal.Booey/DOM/Types/QualifiedName.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/Pair.hpp"

namespace krys::boo::svg
{
  class SVGAnimatedProperty;
  class SVGProperty;

  /// @brief An element's registry of the properties that reflect its attributes.
  class SVGPropertyRegistry
  {
  public:
    SVGPropertyRegistry() noexcept = default;
    virtual ~SVGPropertyRegistry() noexcept = default;

    virtual void DetachAllProperties() const noexcept = 0;
    KRYS_NODISCARD virtual dom::QualifiedName PropertyAttributeName(const SVGProperty &) const noexcept = 0;
    KRYS_NODISCARD virtual dom::QualifiedName
      AnimatedPropertyAttributeName(const SVGAnimatedProperty &) const noexcept = 0;
    virtual void SetAnimatedPropertyDirty(const dom::QualifiedName &,
                                          SVGAnimatedProperty &) const noexcept = 0;
    KRYS_NODISCARD virtual Maybe<dom::DOMString> Synchronize(const dom::QualifiedName &) const noexcept = 0;
    KRYS_NODISCARD virtual List<Pair<dom::QualifiedName, dom::DOMString>>
      SynchronizeAllAttributes() const noexcept = 0;

    KRYS_NODISCARD virtual bool IsAnimatedPropertyAttribute(const dom::QualifiedName &) const noexcept = 0;
    KRYS_NODISCARD virtual bool
      IsAnimatedStylePropertyAttribute(const dom::QualifiedName &) const noexcept = 0;

    // FIXME: createAnimator and appendAnimatedInstance come with SMIL (B11i).
  };
}
