#pragma once

#include <string_view>

namespace Krys
{
  bool is_execution_encoding_utf8() noexcept;
  bool is_execution_encoding_unicode() noexcept;
  bool is_wide_execution_encoding_unicode() noexcept;
}