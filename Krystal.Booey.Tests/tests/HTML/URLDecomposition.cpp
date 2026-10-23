#include "Krystal.Booey/HTML/URLDecomposition.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Loader/NavigationTestUtils.hpp"
#include "Krystal.Booey/HTML/HTMLAnchorElement.hpp"
#include "Krystal.Booey/HTML/HTMLAreaElement.hpp"
#include "Krystal.Booey/URL/URL.hpp"

namespace krys::boo::html::tests
{
  using boo::tests::NavigationFixture;
  using boo::tests::Str;

  TEST_CASE("HTMLHyperlinkElementUtils: the parts of a link's URL", "[HTML][URLDecomposition]")
  {
    NavigationFixture fixture(u8R"html(<a id=relative href=" ../other/file.html?q=1#frag ">x</a>
      <a id=full href="https://user:pw@host.test:8080/p">y</a><a id=none>z</a><a id=bad href="http://[bad">w</a>
      <map><area id=area href="#top"></map><a id=js href="javascript:void(0)">j</a>)html");

    auto &relative = fixture.Get<HTMLAnchorElement>(u8"relative");
    CHECK(Str(relative.Href()) == "https://example.com/other/file.html?q=1#frag");
    CHECK(Str(relative.ToString()) == Str(relative.Href()));
    CHECK(Str(relative.Origin()) == "https://example.com");
    CHECK(Str(relative.Protocol()) == "https:");
    CHECK(Str(relative.Username()).empty());
    CHECK(Str(relative.Password()).empty());
    CHECK(Str(relative.Host()) == "example.com");
    CHECK(Str(relative.Hostname()) == "example.com");
    CHECK(Str(relative.Port()).empty());
    CHECK(Str(relative.Pathname()) == "/other/file.html");
    CHECK(Str(relative.Search()) == "?q=1");
    CHECK(Str(relative.Hash()) == "#frag");

    auto &full = fixture.Get<HTMLAnchorElement>(u8"full");
    CHECK(Str(full.Username()) == "user");
    CHECK(Str(full.Password()) == "pw");
    CHECK(Str(full.Host()) == "host.test:8080");
    CHECK(Str(full.Hostname()) == "host.test");
    CHECK(Str(full.Port()) == "8080");
    CHECK(Str(full.Origin()) == "https://host.test:8080");
    CHECK(Str(full.Search()).empty());
    CHECK(Str(full.Hash()).empty());

    // Without an href, the URL is null.
    auto &none = fixture.Get<HTMLAnchorElement>(u8"none");
    CHECK(Str(none.Href()).empty());
    CHECK(Str(none.Origin()).empty());
    CHECK(Str(none.Protocol()) == ":");

    // A URL that doesn't parse is returned as it's written.
    auto &bad = fixture.Get<HTMLAnchorElement>(u8"bad");
    CHECK(Str(bad.Href()) == "http://[bad");
    CHECK(Str(bad.Origin()).empty());

    CHECK(Str(fixture.Get<HTMLAreaElement>(u8"area").Href()) == "https://example.com/dir/page.html#top");
    CHECK(Str(fixture.Get<HTMLAnchorElement>(u8"js").Protocol()) == "javascript:");
  }

  TEST_CASE("HTMLHyperlinkElementUtils: changing the parts of a link's URL", "[HTML][URLDecomposition]")
  {
    NavigationFixture fixture(
      u8R"(<a id=a href="https://example.com/p?q#h">x</a><a id=bad href="http://[bad">w</a>
      <a id=file href="file:///c:/dir/f.html">f</a><map><area id=area href="/p"></map>)");
    auto &a = fixture.Get<HTMLAnchorElement>(u8"a");
    auto href = [&]
    {
      return Str(a.GetAttribute(u8"href").value_or(dom::DOMString()));
    };

    a.Hash(u8"#x");
    CHECK(href() == "https://example.com/p?q#x");
    a.Hash(u8"y");
    CHECK(href() == "https://example.com/p?q#y");
    a.Hash(u8"");
    CHECK(href() == "https://example.com/p?q");

    // A '#' in the query doesn't start the fragment.
    a.Search(u8"a#b");
    CHECK(href() == "https://example.com/p?a%23b");
    a.Search(u8"");
    CHECK(href() == "https://example.com/p");

    a.Pathname(u8"/n/m");
    CHECK(href() == "https://example.com/n/m");

    a.Port(u8"81");
    CHECK(href() == "https://example.com:81/n/m");
    // The scheme's default port is no port.
    a.Port(u8"443");
    CHECK(href() == "https://example.com/n/m");
    // A port that doesn't start with a digit is ignored.
    a.Port(u8"x1");
    CHECK(href() == "https://example.com/n/m");

    // A special URL keeps its host.
    a.Host(u8"");
    CHECK(href() == "https://example.com/n/m");
    a.Host(u8"other.test:99");
    CHECK(href() == "https://other.test:99/n/m");
    a.Hostname(u8"third.test");
    CHECK(href() == "https://third.test:99/n/m");

    a.Username(u8"me");
    a.Password(u8"secret");
    CHECK(href() == "https://me:secret@third.test:99/n/m");

    a.Protocol(u8"http");
    CHECK(href() == "http://me:secret@third.test:99/n/m");

    // A file URL has no credentials or port.
    auto &file = fixture.Get<HTMLAnchorElement>(u8"file");
    file.Username(u8"me");
    file.Port(u8"80");
    CHECK(Str(file.Href()) == "file:///c:/dir/f.html");

    // An invalid URL keeps its scheme.
    auto &bad = fixture.Get<HTMLAnchorElement>(u8"bad");
    bad.Protocol(u8"https");
    CHECK(Str(bad.GetAttribute(u8"href").value_or(dom::DOMString())) == "http://[bad");

    auto &area = fixture.Get<HTMLAreaElement>(u8"area");
    area.Hash(u8"z");
    CHECK(Str(area.GetAttribute(u8"href").value_or(dom::DOMString())) == "https://example.com/p#z");

    a.Href(dom::USVString(u8"rel/ative"));
    CHECK(href() == "rel/ative");
    CHECK(Str(a.Href()) == "https://example.com/dir/rel/ative");
  }

  TEST_CASE("URLDecomposition::SerializedOrigin", "[HTML][URLDecomposition]")
  {
    auto origin = [](utf8_stringview url)
    {
      return Str(URLDecomposition::SerializedOrigin(url::URL(url)));
    };
    CHECK(origin(u8"https://example.com:8443/a?b#c") == "https://example.com:8443");
    CHECK(origin(u8"http://example.com:80/") == "http://example.com");
    CHECK(origin(u8"ws://[::1]:9000/") == "ws://[::1]:9000");
    CHECK(origin(u8"blob:https://example.com/uuid") == "https://example.com");
    CHECK(origin(u8"blob:data:text/plain,x") == "null");
    CHECK(origin(u8"file:///c:/f.html") == "file://");
    CHECK(origin(u8"data:text/plain,x") == "null");
    CHECK(origin(u8"about:blank") == "null");
  }

  TEST_CASE("URLDecomposition::ParsePort", "[HTML][URLDecomposition]")
  {
    CHECK(URLDecomposition::ParsePort(u8"8080", u8"https") == Maybe<uint16>(8'080));
    CHECK(URLDecomposition::ParsePort(u8"80abc", u8"https") == Maybe<uint16>(80));
    CHECK(URLDecomposition::ParsePort(u8"8\t0", u8"https") == Maybe<uint16>(80));
    // The scheme's default port, and no digits at all, parse as no port.
    auto defaultPort = URLDecomposition::ParsePort(u8"443", u8"https");
    REQUIRE(defaultPort.has_value());
    CHECK_FALSE(defaultPort->has_value());
    auto empty = URLDecomposition::ParsePort(u8"", u8"https");
    REQUIRE(empty.has_value());
    CHECK_FALSE(empty->has_value());
    CHECK_FALSE(URLDecomposition::ParsePort(u8"abc", u8"https").has_value());
    CHECK_FALSE(URLDecomposition::ParsePort(u8"65536", u8"https").has_value());
  }
}
