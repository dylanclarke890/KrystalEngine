#pragma once

#include "Krystal.Booey/SVG/Properties/SVGDecoratedPrimitive.hpp"
#include "Krystal.Core/Concepts.hpp"

namespace krys::boo::svg
{
  /// @brief An enumeration the DOM sees as an integer, refusing values outside the enumeration and reporting
  /// those the DOM doesn't expose as 0 (unknown).
  template <typename TDecoration, typename TEnum>
  class SVGDecoratedEnumeration : public SVGDecoratedPrimitive<TDecoration, TEnum>
  {
    using Base = SVGDecoratedPrimitive<TDecoration, TEnum>;
    using Base::_value;
    using Base::Base;

    static constexpr TDecoration OutOfRangeEnumValue = 0;

  public:
    KRYS_NODISCARD static auto Create(const TEnum &value) noexcept
    {
      static_assert(Integral<TDecoration>, "TDecoration of an enumeration should be integral.");
      return AdoptRef(*new SVGDecoratedEnumeration(value));
    }

  private:
    bool SetValue(const TDecoration &value) noexcept override
    {
      if (!value || value > SVGPropertyTraits<TEnum>::HighestEnumValue())
      {
        return false;
      }
      Base::SetValueInternal(value);
      return true;
    }

    KRYS_NODISCARD TDecoration Value() const noexcept override
    {
      if (Base::Value() > SVGPropertyTraits<TEnum>::HighestEnumValue())
      {
        return OutOfRangeEnumValue;
      }
      return Base::Value();
    }

    KRYS_NODISCARD Ref<SVGDecoratedProperty<TDecoration>> Clone() noexcept override
    {
      return Create(_value);
    }
  };
}
