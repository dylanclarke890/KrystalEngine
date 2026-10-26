#pragma once

#include "Krystal.Booey/SVG/Properties/SVGDecoratedProperty.hpp"
#include "Krystal.Booey/SVG/Properties/SVGPropertyTraits.hpp"

namespace krys::boo::svg
{
  /// @brief A decorated property kept as a `TProperty` and converted to and from `TDecoration` by a cast.
  template <typename TDecoration, typename TProperty>
  class SVGDecoratedPrimitive : public SVGDecoratedProperty<TDecoration>
  {
    using Base = SVGDecoratedProperty<TDecoration>;

  protected:
    TProperty _value;

  public:
    SVGDecoratedPrimitive(const TProperty &value) noexcept : _value(value)
    {
    }

  protected:
    using Base::Base;

    void SetValueInternal(const TDecoration &value) noexcept override
    {
      _value = static_cast<TProperty>(value);
    }

    KRYS_NODISCARD TDecoration ValueInternal() const noexcept override
    {
      return static_cast<TDecoration>(_value);
    }

    KRYS_NODISCARD dom::DOMString ValueAsString() const noexcept override
    {
      return SVGPropertyTraits<TProperty>::ToString(_value);
    }
  };
}
