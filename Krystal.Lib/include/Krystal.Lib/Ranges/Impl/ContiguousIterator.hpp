#pragma once

#include "Krystal.Lib/Core/Concepts.hpp"
#include "Krystal.Lib/Core/TypeTraits.hpp"
#include "Krystal.Lib/Detection/Compiler.hpp"

namespace Krys::Ranges
{
  namespace Impl
  {
    template <typename TIterator, typename = void>
    struct ContiguousIterator : public IntegralConstant<bool, IsPointer<remove_cvref_t<TIterator>>>
    {
    };

#if KRYS_COMPILER_STL(GCC)
    template <typename TIterator, typename TParent>
    struct ContiguousIterator<::__gnu_cxx::__normal_iterator<TIterator, TParent>>
        : public ContiguousIterator<TIterator>
    {
    };
#endif

#if KRYS_COMPILER_STL(MSVC)
    template <typename TIterator>
    struct ContiguousIterator<TIterator, void_t<decltype(std::declval<TIterator>()._Unwrapped())>>
        : public ContiguousIterator<decltype(std::declval<TIterator>()._Unwrapped())>
    {
    };
#endif

#if KRYS_COMPILER_STL(CLANG)
    template <typename TIterator>
    struct ContiguousIterator<std::__wrap_iter<TIterator>> : public ContiguousIterator<TIterator>
    {
    };
#endif

    template <typename TIterator>
    concept IsContiguousIterator = Impl::ContiguousIterator<TIterator>::value;
  }
}