#pragma once

#include "Krystal.HTML/CSS/Values/Enums/CSSUnitCategory.hpp"
#include "Krystal.HTML/CSS/Values/Enums/CSSUnitType.hpp"
#include "Krystal.HTML/CSS/Values/Enums/CSSValueId.hpp"
#include "Krystal.HTML/CSS/Values/Enums/CSSValueType.hpp"
#include "Krystal.Lib/Mixins/RefCounted.hpp"

namespace Krys::HTML
{
  class CSSValue : public RefCounted<CSSValue>
  {
  private:
    CSSValueType _type : BitCount<CSSValueType>();
    CSSUnitType _unit : BitCount<CSSUnitType>();

  protected:
    CSSValue(CSSValueType type) noexcept : _type(type)
    {
    }

  public:
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
  };
}