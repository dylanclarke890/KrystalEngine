#pragma once

#include "Krystal.Booey/CSS/Values/ValueList.hpp"
#include "Krystal.Booey/CSS/Values/ValueListBuilder.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::css
{
  enum class ValueId : uint16;

  /// @brief A function with comma-separated arguments, e.g. `translate(10px, 20px)`.
  class FunctionValue final : public ValueContainingList
  {
    friend class Value;

  private:
    css::ValueId _name {};

    FunctionValue(css::ValueId name, ValueListBuilder arguments) noexcept;
    explicit FunctionValue(css::ValueId name) noexcept;
    FunctionValue(css::ValueId name, Ref<Value> argument) noexcept;
    FunctionValue(css::ValueId name, Ref<Value> argument1, Ref<Value> argument2) noexcept;
    FunctionValue(css::ValueId name, Ref<Value> argument1, Ref<Value> argument2,
                  Ref<Value> argument3) noexcept;
    FunctionValue(css::ValueId name, Ref<Value> argument1, Ref<Value> argument2, Ref<Value> argument3,
                  Ref<Value> argument4) noexcept;

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

    KRYS_NODISCARD CSSOMString CustomCSSText(const SerialisationContext &context) const noexcept;

    KRYS_NODISCARD bool Equals(const FunctionValue &other) const noexcept
    {
      return _name == other._name && ItemsEqual(other);
    }

  private:
    KRYS_NODISCARD bool AddDerivedHash(size_t &seed) const noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::FunctionValue)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::Value &value) noexcept
  {
    return value.IsFunctionValue();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
