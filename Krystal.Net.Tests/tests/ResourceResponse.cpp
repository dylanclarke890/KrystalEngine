#include "Krystal.Net/ResourceResponse.hpp"
#include "catch2/catch_test_macros.hpp"

namespace krys::net::tests
{
  namespace
  {
    using namespace std::chrono;

    /// @brief 1994-11-06 08:49:37 UTC, the HTTP RFC's example date.
    constexpr sys_seconds LastModifiedExample = sys_days(1'994y / November / 6) + 8h + 49min + 37s;
  }

  TEST_CASE("ResourceResponse: a status, headers and the Last-Modified date",
            "[Platform][Network][ResourceResponse]")
  {
    ResourceResponse response(net::Url(u8"https://example.com/"), u8"text/html", 0);
    CHECK(response.HttpStatusCode() == 0);
    CHECK_FALSE(response.IsSuccessful());
    CHECK_FALSE(response.LastModified());
    CHECK(response.HttpHeaderField(u8"Last-Modified").empty());

    response.SetHttpStatusCode(204);
    CHECK(response.IsSuccessful());
    response.SetHttpStatusCode(404);
    CHECK_FALSE(response.IsSuccessful());

    response.SetHttpHeaderField(u8"last-modified", u8"Sun, 06 Nov 1994 08:49:37 GMT");
    CHECK(response.LastModified() == LastModifiedExample);
    response.SetHttpHeaderField(u8"Last-Modified", u8"yesterday");
    CHECK_FALSE(response.LastModified());
  }
}
