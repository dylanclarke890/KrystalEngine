#pragma once

#include "Krystal.Booey/CSS/Values/Value.hpp"
#include "Krystal.Booey/CSS/Values/ValueList.hpp"
#include "Krystal.Booey/CSS/Values/ValueListBuilder.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::css
{
  class FunctionValue final : public ValueWithSmallList
  {
  private:
    css::ValueId _name;

  public:
    KRYS_NODISCARD static Ref<FunctionValue> Create(css::ValueId name, ValueListBuilder arguments) noexcept;

    KRYS_NODISCARD static Ref<FunctionValue> Create(css::ValueId name) noexcept;

    KRYS_NODISCARD static Ref<FunctionValue> Create(css::ValueId name, Ref<Value> argument) noexcept;

    KRYS_NODISCARD static Ref<FunctionValue> Create(css::ValueId name, Ref<Value> firstArgument,
                                                    Ref<Value> secondArgument) noexcept;

    KRYS_NODISCARD static Ref<FunctionValue> Create(css::ValueId name, Ref<Value> firstArgument,
                                                    Ref<Value> secondArgument,
                                                    Ref<Value> thirdArgument) noexcept;

    KRYS_NODISCARD static Ref<FunctionValue> Create(css::ValueId name, Ref<Value> firstArgument,
                                                    Ref<Value> secondArgument, Ref<Value> thirdArgument,
                                                    Ref<Value> fourthArgument) noexcept;

    KRYS_NODISCARD css::ValueId Name() const noexcept
    {
      return _name;
    }

    KRYS_NODISCARD size_t Length() const noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::FunctionValue)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::Value &value) noexcept
  {
    return value.IsFunctionValue();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()