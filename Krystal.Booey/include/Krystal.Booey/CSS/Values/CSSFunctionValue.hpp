#pragma once

#include "Krystal.Booey/CSS/Values/CSSValue.hpp"
#include "Krystal.Booey/CSS/Values/CSSValueList.hpp"
#include "Krystal.Booey/CSS/Values/CSSValueListBuilder.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::css
{
  class CSSFunctionValue final : public CSSValueContainingVector
  {
  private:
    css::ValueId _name;

  public:
    KRYS_NODISCARD static Ref<CSSFunctionValue> Create(css::ValueId name,
                                                       CSSValueListBuilder arguments) noexcept;

    KRYS_NODISCARD static Ref<CSSFunctionValue> Create(css::ValueId name) noexcept;

    KRYS_NODISCARD static Ref<CSSFunctionValue> Create(css::ValueId name, Ref<CSSValue> argument) noexcept;

    KRYS_NODISCARD static Ref<CSSFunctionValue> Create(css::ValueId name, Ref<CSSValue> firstArgument,
                                                       Ref<CSSValue> secondArgument) noexcept;

    KRYS_NODISCARD static Ref<CSSFunctionValue> Create(css::ValueId name, Ref<CSSValue> firstArgument,
                                                       Ref<CSSValue> secondArgument,
                                                       Ref<CSSValue> thirdArgument) noexcept;

    KRYS_NODISCARD static Ref<CSSFunctionValue> Create(css::ValueId name, Ref<CSSValue> firstArgument,
                                                       Ref<CSSValue> secondArgument,
                                                       Ref<CSSValue> thirdArgument,
                                                       Ref<CSSValue> fourthArgument) noexcept;

    KRYS_NODISCARD css::ValueId Name() const noexcept
    {
      return _name;
    }

    KRYS_NODISCARD size_t Length() const noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::CSSFunctionValue)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::CSSValue &value) noexcept
  {
    return value.IsCSSFunctionValue();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()