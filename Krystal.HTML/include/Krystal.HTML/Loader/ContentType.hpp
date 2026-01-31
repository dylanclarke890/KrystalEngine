#pragma once

#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Core/Enum.hpp"
#include "Krystal.Lib/String/String.hpp"
#include "Krystal.Lib/Types/Numeric.hpp"
#include "Krystal.Text/ASCII.hpp"

namespace Krys::HTML
{
  enum class ContentType : uint8
  {
    /// @brief Checks only for BOM.
    PlainText,
    HTML,
    CSS
  };

  constexpr KRYS_NODISCARD ContentType DetermineContentType(const string &mimeType) noexcept
  {
    // TODO: should be a caseless comparison.

    if (mimeType == "text/css")
    {
      return ContentType::CSS;
    }

    if (mimeType == "text/html")
    {
      return ContentType::HTML;
    }

    return ContentType::PlainText;
  }
}

KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(Krys::HTML::ContentType, 3u)
