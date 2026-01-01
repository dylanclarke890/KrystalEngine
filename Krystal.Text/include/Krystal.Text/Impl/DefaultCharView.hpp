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
  class DefaultCharView
  {
  public:
    using type = Span<TChar>;
  };

  template <typename TChar>
  class DefaultCharView<TChar, true>
  {
  public:
    using type = std::basic_string_view<TChar>;
  };

  template <typename TChar>
  using default_char_view_t = typename DefaultCharView<TChar>::type;
}
