#pragma once

#include "Krystal.Core/Base.hpp"

namespace krys::boo::html
{
  enum class ContentType : uint8
  {
    /// @brief Checks only for BOM.
    PlainText,
    HTML,
    CSS
  };

  KRYS_NODISCARD inline ContentType DetermineContentType(const string &mimeType) noexcept
  {
    string lowerMimeType = krys::text::ToASCIILower(mimeType);

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

KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(krys::boo::html::ContentType, 3u)
