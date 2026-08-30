#pragma once

#include "Krystal.HTML/CSS/Values/CSSValue.hpp"
#include "Krystal.HTML/CSS/Values/CSSValueList.hpp"
#include "Krystal.HTML/CSS/Values/CSSValueListBuilder.hpp"
#include "Krystal.Lib/Pointers/RefPtr.hpp"

namespace Krys::HTML
{
  class CSSFunctionValue final : public CSSValueContainingVector
  {
  private:
    CSSValueId _name;

  public:
    KRYS_NODISCARD static Ref<CSSFunctionValue> Create(CSSValueId name,
                                                       CSSValueListBuilder arguments) noexcept;

    KRYS_NODISCARD static Ref<CSSFunctionValue> Create(CSSValueId name) noexcept;

    KRYS_NODISCARD static Ref<CSSFunctionValue> Create(CSSValueId name, Ref<CSSValue> argument) noexcept;

    KRYS_NODISCARD static Ref<CSSFunctionValue> Create(CSSValueId name, Ref<CSSValue> firstArgument,
                                                       Ref<CSSValue> secondArgument) noexcept;

    KRYS_NODISCARD static Ref<CSSFunctionValue> Create(CSSValueId name, Ref<CSSValue> firstArgument,
                                                       Ref<CSSValue> secondArgument,
                                                       Ref<CSSValue> thirdArgument) noexcept;

    KRYS_NODISCARD static Ref<CSSFunctionValue> Create(CSSValueId name, Ref<CSSValue> firstArgument,
                                                       Ref<CSSValue> secondArgument,
                                                       Ref<CSSValue> thirdArgument,
                                                       Ref<CSSValue> fourthArgument) noexcept;

    KRYS_NODISCARD CSSValueId Name() const noexcept
    {
      return _name;
    }

    KRYS_NODISCARD size_t Length() const noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(Krys::HTML::CSSFunctionValue)
  KRYS_NODISCARD static bool IsType(const Krys::HTML::CSSValue &value) noexcept
  {
    return value.IsCSSFunctionValue();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()