#pragma once

#include "Krystal.Booey/SVG/Properties/SVGProperty.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::svg
{
  /// @brief An SVG property holding a value of `TProperty`.
  template <typename TProperty>
  class SVGValueProperty : public SVGProperty
  {
  protected:
    TProperty _value;

  public:
    using ValueType = TProperty;

    KRYS_NODISCARD static Ref<SVGValueProperty> Create() noexcept
    {
      return AdoptRef(*new SVGValueProperty());
    }

    // Getter/Setter for the value.
    KRYS_NODISCARD const TProperty &Value() const noexcept
    {
      return _value;
    }

    void SetValue(const TProperty &value) noexcept
    {
      _value = value;
    }

    // Used by the SVGAnimatedPropertyAnimator to pass _value to SVGAnimationFunction.
    KRYS_NODISCARD TProperty &Value() noexcept
    {
      return _value;
    }

  protected:
    // Create an initialized property, e.g creating an item to be appended in an SVGList.
    SVGValueProperty(const TProperty &value) noexcept : _value(value)
    {
    }

    // Needed when value should not be copied, e.g. SVGTransformValue.
    SVGValueProperty(TProperty &&value) noexcept : _value(krys::move(value))
    {
    }

    // Base and default constructor.
    SVGValueProperty(SVGPropertyOwner *owner = nullptr,
                     SVGPropertyAccess access = SVGPropertyAccess::ReadWrite) noexcept
        : SVGProperty(owner, access)
    {
    }

    // Create an initialized and attached property.
    SVGValueProperty(SVGPropertyOwner *owner, SVGPropertyAccess access, const TProperty &value) noexcept
        : SVGProperty(owner, access), _value(value)
    {
    }
  };
}
