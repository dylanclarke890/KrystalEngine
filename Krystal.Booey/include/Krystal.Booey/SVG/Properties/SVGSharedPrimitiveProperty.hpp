#pragma once

#include "Krystal.Booey/SVG/Properties/SVGProperty.hpp"
#include "Krystal.Booey/SVG/Properties/SVGPropertyTraits.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::svg
{
  /// @brief The value of an animated primitive property (a boolean, integer, number or string).
  template <typename TProperty>
  class SVGSharedPrimitiveProperty : public SVGProperty
  {
  private:
    TProperty _value;

  public:
    using ValueType = TProperty;

    KRYS_NODISCARD static Ref<SVGSharedPrimitiveProperty>
      Create(const TProperty &value = SVGPropertyTraits<TProperty>::InitialValue()) noexcept
    {
      return AdoptRef(*new SVGSharedPrimitiveProperty(value));
    }

    // Getter/Setter for the value.
    KRYS_NODISCARD const TProperty &Value() const noexcept
    {
      return _value;
    }

    KRYS_NODISCARD TProperty &Value() noexcept
    {
      return _value;
    }

    void SetValue(const TProperty &value) noexcept
    {
      _value = value;
    }

    KRYS_NODISCARD dom::DOMString ValueAsString() const noexcept override
    {
      return SVGPropertyTraits<TProperty>::ToString(_value);
    }

  private:
    SVGSharedPrimitiveProperty(const TProperty &value) noexcept : _value(value)
    {
    }
  };
}
