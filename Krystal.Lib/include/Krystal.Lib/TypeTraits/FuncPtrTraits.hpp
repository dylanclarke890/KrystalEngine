#pragma once

#include <tuple>

namespace Krys
{
  template <typename>
  struct FuncPtrTraits;

  template <typename R, typename... Args>
  struct FuncPtrTraits<R (*)(Args...)>
  {
    using ReturnType = R;
    using ArgsTuple = std::tuple<Args...>;
    static constexpr size_t NumArgs = sizeof...(Args);
  };
}