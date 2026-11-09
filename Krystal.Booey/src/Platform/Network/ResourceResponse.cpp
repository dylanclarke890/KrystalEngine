#include "Krystal.Booey/Platform/Network/ResourceResponse.hpp"
#include "Krystal.Booey/Platform/Network/HTTPParsers.hpp"

namespace krys::boo
{
  Maybe<std::chrono::sys_seconds> ResourceResponse::LastModified() const noexcept
  {
    auto value = _httpHeaderFields.Get(u8"Last-Modified");
    return value ? ParseHTTPDate(*value) : null;
  }
}
