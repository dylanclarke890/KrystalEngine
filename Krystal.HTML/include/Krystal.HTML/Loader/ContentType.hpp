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
}

KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(Krys::HTML::ContentType, 3u)

namespace Krys::HTML
{
  KRYS_NODISCARD inline ContentType DetermineContentType(const string &mimeType) noexcept
  {
    string lowerMimeType = Text::ToASCIILowercase(mimeType);

    if (lowerMimeType == "text/css")
    {
      return ContentType::CSS;
    }

    if (lowerMimeType == "text/html")
    {
      return ContentType::HTML;
    }

    return ContentType::PlainText;
  }
}
