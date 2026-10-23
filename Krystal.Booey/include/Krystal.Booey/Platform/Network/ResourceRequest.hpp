#pragma once

#include "Krystal.Booey/URL/URL.hpp"
#include "Krystal.Core/Attributes.hpp"

namespace krys::boo
{
  /// @brief What's asked of the loader: the URL to load, and the HTTP method.
  // FIXME: The headers, body, cache policy, priority and first party, once there are network loads.
  class ResourceRequest
  {
  private:
    url::URL _url;
    utf8_string _httpMethod {u8"GET"};

  public:
    ResourceRequest() noexcept = default;
    explicit ResourceRequest(url::URL &&url) noexcept : _url(krys::move(url))
    {
    }
    explicit ResourceRequest(const url::URL &url) noexcept : _url(url)
    {
    }

    KRYS_NODISCARD const url::URL &URL() const noexcept
    {
      return _url;
    }
    void SetURL(url::URL &&url) noexcept
    {
      _url = krys::move(url);
    }
    void RemoveFragmentIdentifier() noexcept
    {
      _url.RemoveFragmentIdentifier();
    }

    KRYS_NODISCARD const utf8_string &HTTPMethod() const noexcept
    {
      return _httpMethod;
    }
    void SetHTTPMethod(utf8_string &&method) noexcept
    {
      _httpMethod = krys::move(method);
    }

    KRYS_NODISCARD bool IsNull() const noexcept
    {
      return _url.IsNull();
    }
  };
}
