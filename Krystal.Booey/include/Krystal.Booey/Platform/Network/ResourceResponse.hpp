#pragma once

#include "Krystal.Booey/Platform/Network/HTTPHeaderMap.hpp"
#include "Krystal.Booey/URL/URL.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/String.hpp"
#include <chrono>

namespace krys::boo
{
  /// @brief What came back for a load: the URL it came from, its type and the length it said it had, and for
  /// an HTTP load, its status and headers. The host's loader strategy fills them in; a file load may give a
  /// `Last-Modified` header of the file's modification time.
  class ResourceResponse
  {
  private:
    url::URL _url;
    utf8_string _mimeType;
    int64 _expectedContentLength {0};
    utf8_string _textEncodingName;
    uint16 _httpStatusCode {0};
    utf8_string _httpStatusText;
    HTTPHeaderMap _httpHeaderFields;

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
    void SetURL(url::URL url) noexcept
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

    /// @brief The HTTP status code; 0 for a response that isn't HTTP's (a file's, a `data:` URL's).
    KRYS_NODISCARD uint16 HTTPStatusCode() const noexcept
    {
      return _httpStatusCode;
    }
    void SetHTTPStatusCode(uint16 statusCode) noexcept
    {
      _httpStatusCode = statusCode;
    }
    KRYS_NODISCARD const utf8_string &HTTPStatusText() const noexcept
    {
      return _httpStatusText;
    }
    void SetHTTPStatusText(utf8_string statusText) noexcept
    {
      _httpStatusText = krys::move(statusText);
    }
    /// @brief Whether the status is a success, 200 to 299.
    /// @see https://fetch.spec.whatwg.org/#ok-status
    KRYS_NODISCARD bool IsSuccessful() const noexcept
    {
      return _httpStatusCode >= 200 && _httpStatusCode <= 299;
    }

    KRYS_NODISCARD const HTTPHeaderMap &HTTPHeaderFields() const noexcept
    {
      return _httpHeaderFields;
    }
    /// @brief The value of the header @p name; empty without one.
    KRYS_NODISCARD utf8_stringview HTTPHeaderField(utf8_stringview name) const noexcept
    {
      return _httpHeaderFields.Get(name).value_or(utf8_stringview());
    }
    void SetHTTPHeaderField(utf8_stringview name, utf8_stringview value) noexcept
    {
      _httpHeaderFields.Set(name, value);
    }
    void AddHTTPHeaderField(utf8_stringview name, utf8_stringview value) noexcept
    {
      _httpHeaderFields.Add(name, value);
    }

    /// @brief The time its `Last-Modified` header names, if it has a valid one.
    KRYS_NODISCARD Maybe<std::chrono::sys_seconds> LastModified() const noexcept;
  };
}
