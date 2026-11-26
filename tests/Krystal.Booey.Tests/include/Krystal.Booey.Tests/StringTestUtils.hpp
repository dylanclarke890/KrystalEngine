#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Strings/String.hpp"
#include <string>

namespace krys::boo::tests
{
  /// @brief @p text as a `std::string`, which Catch2 can print.
  KRYS_NODISCARD inline std::string Narrow(utf8_stringview text) noexcept
  {
    return std::string(reinterpret_cast<const char *>(text.data()), text.size());
  }

  /// @brief Whether @p text contains @p part.
  KRYS_NODISCARD inline bool Contains(utf8_stringview text, utf8_stringview part) noexcept
  {
    return text.contains(part);
  }

  /// @brief How many times @p part occurs in @p text, overlaps included.
  KRYS_NODISCARD inline size_t Count(utf8_stringview text, utf8_stringview part) noexcept
  {
    size_t count = 0;
    for (auto position = text.find(part); position != utf8_stringview::npos;
         position = text.find(part, position + 1))
    {
      ++count;
    }
    return count;
  }
}
