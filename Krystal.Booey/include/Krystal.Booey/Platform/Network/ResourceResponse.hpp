#pragma once

#include "Krystal.Booey/URL/URL.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/Types/String.hpp"

namespace krys::boo
{
  /// @brief What came back for a load: the URL it came from, its type and the length it said it had.
  // FIXME: The HTTP status, headers, cache control, tainting and timing, once there are network loads.
  class ResourceResponse
  {
  private:
    url::URL _url;
    utf8_string _mimeType;
    int64 _expectedContentLength {0};
    utf8_string _textEncodingName;

  public:
    ResourceResponse() noexcept = default;
    ResourceResponse(url::URL url, utf8_string mimeType, int64 expectedContentLength,
                     utf8_string textEncodingName = {}) noexcept
        : _url(krys::move(url)), _mimeType(krys::move(mimeType)),
          _expectedContentLength(expectedContentLength), _textEncodingName(krys::move(textEncodingName))
    {
    }

    KRYS_NODISCARD bool IsNull() const noexcept
    {
      return _url.IsNull();
    }

    KRYS_NODISCARD const url::URL &URL() const noexcept
    {
      return _url;
    }
    void SetURL(url::URL &&url) noexcept
    {
      _url = krys::move(url);
    }

    KRYS_NODISCARD const utf8_string &MimeType() const noexcept
    {
      return _mimeType;
    }
    KRYS_NODISCARD int64 ExpectedContentLength() const noexcept
    {
      return _expectedContentLength;
    }
    KRYS_NODISCARD const utf8_string &TextEncodingName() const noexcept
    {
      return _textEncodingName;
    }
  };
}
