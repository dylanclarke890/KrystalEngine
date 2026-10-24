#pragma once

#include "Krystal.Booey/URL/URL.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/String.hpp"

namespace krys::boo
{
  /// @brief A document's content the host gives directly, in place of loading the request's URL: markup and
  /// the type it's in.
  class SubstituteData
  {
  private:
    utf8_string _content;
    utf8_string _mimeType {u8"text/html"};
    url::URL _responseURL;

  public:
    SubstituteData() noexcept = default;

    /// @param responseURL The document's URL; the request's if null.
    SubstituteData(utf8_string &&content, utf8_string &&mimeType, url::URL &&responseURL) noexcept
        : _content(krys::move(content)), _mimeType(krys::move(mimeType)),
          _responseURL(krys::move(responseURL))
    {
    }

    KRYS_NODISCARD const utf8_string &Content() const noexcept
    {
      return _content;
    }

    KRYS_NODISCARD const utf8_string &MimeType() const noexcept
    {
      return _mimeType;
    }

    KRYS_NODISCARD const url::URL &ResponseURL() const noexcept
    {
      return _responseURL;
    }
  };
}
