#pragma once

#include "Krystal.Lib/Types/Numeric.hpp"
#include "Krystal.Text/InlineContainers/InlineBasicString.hpp"

namespace Krys
{
  template <std::size_t Capacity>
  using InlineString = InlineBasicString<char, Capacity>;

  template <std::size_t Capacity>
  using InlineWString = InlineBasicString<wchar, Capacity>;

  template <std::size_t Capacity>
  using InlineUTF8String = InlineBasicString<uchar8, Capacity>;

  template <std::size_t Capacity>
  using InlineUTF16string = InlineBasicString<uchar16, Capacity>;

  template <std::size_t Capacity>
  using InlineUTF32string = InlineBasicString<uchar32, Capacity>;
}