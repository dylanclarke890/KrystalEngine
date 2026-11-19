#include "Krystal.Booey/Platform/Network/ResourceResponse.hpp"
#include "catch2/catch_test_macros.hpp"

namespace krys::boo::tests
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
    ResourceResponse response(url::URL(u8"https://example.com/"), u8"text/html", 0);
    CHECK(response.HTTPStatusCode() == 0);
    CHECK_FALSE(response.IsSuccessful());
    CHECK_FALSE(response.LastModified());
    CHECK(response.HTTPHeaderField(u8"Last-Modified").empty());

    response.SetHTTPStatusCode(204);
    CHECK(response.IsSuccessful());
    response.SetHTTPStatusCode(404);
    CHECK_FALSE(response.IsSuccessful());

    response.SetHTTPHeaderField(u8"last-modified", u8"Sun, 06 Nov 1994 08:49:37 GMT");
    CHECK(response.LastModified() == LastModifiedExample);
    response.SetHTTPHeaderField(u8"Last-Modified", u8"yesterday");
    CHECK_FALSE(response.LastModified());
  }
}
