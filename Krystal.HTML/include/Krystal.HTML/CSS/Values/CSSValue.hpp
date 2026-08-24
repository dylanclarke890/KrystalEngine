#pragma once

#include "Krystal.HTML/CSS/Values/Enums/CSSUnitCategory.hpp"
#include "Krystal.HTML/CSS/Values/Enums/CSSUnitType.hpp"
#include "Krystal.HTML/CSS/Values/Enums/CSSValueId.hpp"
#include "Krystal.HTML/CSS/Values/Enums/CSSValueType.hpp"
#include "Krystal.Lib/Core/TypeCast.hpp"
#include "Krystal.Lib/Mixins/RefCounted.hpp"

namespace Krys::HTML
{
  class CSSValue : public RefCounted<CSSValue>
  {
    KRYS_TYPE_CAST_TRAITS_ACCESS();

  protected:
    CSSValueType _type : BitCount<CSSValueType>();
    CSSUnitType _unit : BitCount<CSSUnitType>() {CSSUnitType::Unknown};
    bool _isImplicitInitialValue : 1 {false};

    CSSValue(CSSValueType type) noexcept : _type(type)
    {
    }

    struct StaticCSSValueTag
    {
    };

    constexpr inline static StaticCSSValueTag StaticCSSValue {};

  public:
    virtual ~CSSValue() noexcept = default;

    KRYS_NODISCARD CSSValueId ValueId() const noexcept;

    KRYS_NODISCARD bool IsImplicitInitialValue() const noexcept
    {
      return _isImplicitInitialValue;
    }

    KRYS_NODISCARD static bool IsCSSWideKeyword(CSSValueId id) noexcept
    {
      switch (id)
      {
        case CSSValueId::Initial:
        case CSSValueId::Inherit:
        case CSSValueId::Unset:
        case CSSValueId::Revert:
        case CSSValueId::RevertLayer:
        {
          return true;
        }
        default:
        {
          return false;
        }
      }
    }

#pragma region Type Checks

    KRYS_NODISCARD bool IsCSSPrimitiveValue() const noexcept
    {
      return _type == CSSValueType::Primitive;
    }

#pragma endregion
  };
}