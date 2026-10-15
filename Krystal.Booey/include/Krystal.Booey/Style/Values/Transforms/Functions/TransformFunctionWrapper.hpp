#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Utils/Move.hpp"

namespace krys::boo::style
{
  /// @brief Holds a shared, immutable transform function and compares by value.
  template <typename Function>
  struct TransformFunctionWrapper
  {
    Ref<const Function> Value;

    TransformFunctionWrapper(Ref<const Function> value) noexcept : Value {krys::move(value)}
    {
    }

    KRYS_NODISCARD const Function &GetFunction() const noexcept
    {
      return *Value;
    }
    KRYS_NODISCARD const Function *operator->() const noexcept
    {
      return Value.get();
    }

    KRYS_NODISCARD bool operator==(const TransformFunctionWrapper &other) const noexcept
    {
      return Value.get() == other.Value.get() || *Value == *other.Value;
    }
  };
}
