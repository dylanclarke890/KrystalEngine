#pragma once

#include "Krystal.Lib/Core/Concepts.hpp"
#include "Krystal.Lib/Types/Numeric.hpp"

namespace Krys
{
  template <typename T>
  concept SupportsCheckedPtr = requires(T &value) {
    { value.AddRefChecked() } -> SameType<void>;
    { value.SubRefChecked() } -> SameType<void>;
    { value.GetRefCountChecked() } -> ConvertibleTo<uint32>;
  };

  template <typename T>
  concept SupportsRefPtr = requires(T &value) {
    { value.AddRef() } -> SameType<void>;
    { value.SubRef() } -> SameType<void>;
    { value.GetRefCount() } -> ConvertibleTo<uint32>;
  };
}