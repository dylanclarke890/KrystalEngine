#pragma once

#include "Krystal.Lib/Core/TypeTraits.hpp"
#include "Krystal.Lib/Types/Span.hpp"
#include "Krystal.Text/CharTraits.hpp"
#include "Krystal.Text/TypeTraits.hpp"
#include <string>
#include <string_view>
#include <vector>

namespace Krys::Impl
{
  template <typename TChar, bool = IsCharTraitable<TChar>>
  class DefaultCharRange
  {
  public:
    using type = std::vector<TChar>;
  };

  template <typename TChar>
  class DefaultCharRange<TChar, true>
  {
  public:
    using type = std::basic_string<TChar>;
  };

  template <typename TChar>
  using default_char_range_t = typename DefaultCharRange<TChar>::type;
}
