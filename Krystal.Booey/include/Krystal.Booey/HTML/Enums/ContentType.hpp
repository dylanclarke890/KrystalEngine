#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Enum.hpp"
#include "Krystal.Core/Types/String.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Text/ASCII.hpp"

namespace krys::boo::html
{
  enum class ContentType : uint8
  {
    /// @brief Checks only for BOM.
    PlainText,
    HTML,
    CSS
  };
}

KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(krys::boo::html::ContentType, 3u)

namespace krys::boo::html
{
  KRYS_NODISCARD inline ContentType DetermineContentType(const string &mimeType) noexcept
  {
    string lowerMimeType = krys::Text::ToASCIILowercase(mimeType);

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
