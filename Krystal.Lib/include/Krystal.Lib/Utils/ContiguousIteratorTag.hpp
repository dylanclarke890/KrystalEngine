#pragma once

#include "Krystal.Lib/Core/Config.hpp"
#include <iterator>

namespace Krys
{
  namespace Impl
  {
    class ContiguousIteratorTag : public std::random_access_iterator_tag
    {
    };
  }
  
  /// @brief Either a typedef or a polyfill of the contiguous iterator tag, only standardized in C++20.
  using contiguous_iterator_tag =
#if KRYS_CONFIG(STD_LIBRARY_CONTIGUOUS_ITERATOR_TAG)
    std::contiguous_iterator_tag;
#else
    Impl::ContiguousIteratorTag;
#endif
}
