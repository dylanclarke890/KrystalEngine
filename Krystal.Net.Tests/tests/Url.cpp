#include "Krystal.Net/URL.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Core/Utils/Move.hpp"
#include "Krystal.Net/UrlHash.hpp"
#include <string>

namespace krys::net::tests
{
  namespace
  {
    std::string S(utf8_stringview value)
    {
      return std::string(reinterpret_cast<const char *>(value.data()), value.size());
    }

    Url CreateUrl(utf8_stringview string)
    {
      return Url(string);
    }
  }

  TEST_CASE("URL(default)", "[URL]")
  {
    Url url;

    CHECK(url.IsEmpty());
    CHECK(url.IsNull());
    CHECK_FALSE(url.IsValid());
  }

  TEST_CASE("URL(components)", "[URL]")
  {
    Url url {u8"http://username:password@www.example.com:8080/index.html?var=val#fragment"};

    CHECK_FALSE(url.IsEmpty());
    CHECK_FALSE(url.IsNull());
    CHECK(url.IsValid());

    CHECK(S(url.Protocol()) == "http");
    CHECK(S(url.Host()) == "www.example.com");
    REQUIRE(url.Port());
    CHECK(*url.Port() == 8'080);
    CHECK(S(url.User()) == "username");
    CHECK(S(url.Password()) == "password");
    CHECK(S(url.Path()) == "/index.html");
    CHECK(S(url.LastPathComponent()) == "index.html");
    CHECK(S(url.Query()) == "var=val");
    CHECK(url.HasFragmentIdentifier());
    CHECK(S(url.FragmentIdentifier()) == "fragment");
  }

  TEST_CASE("URL(ProtocolHostAndPort)", "[URL]")
  {
    struct TestCase
    {
      utf8_stringview Input;
      const char *Expected;
    };

    TestCase cases[] {
      {u8"http://username:password@www.example.com:8080/index.html?var=val#fragment",
       "http://www.example.com:8080"},
      {u8"http://username:@www.example.com:8080/index.html?var=val#fragment", "http://www.example.com:8080"},
      {u8"http://:password@www.example.com:8080/index.html?var=val#fragment", "http://www.example.com:8080"},
      {u8"http://username@www.example.com:8080/index.html?var=val#fragment", "http://www.example.com:8080"},
      {u8"http://www.example.com:8080/index.html?var=val#fragment", "http://www.example.com:8080"},
      {u8"http://www.example.com:/index.html?var=val#fragment", "http://www.example.com"},
      {u8"http://www.example.com/index.html?var=val#fragment", "http://www.example.com"},
      {u8"file:///a/b/c", "file://"},
      {u8"file:///a/b", "file://"},
      {u8"file:///a", "file://"},
      {u8"asdf://username:password@www.example.com:8080/index.html?var=val#fragment",
       "asdf://www.example.com:8080"},
      {u8"asdf:///a/b/c", "asdf://"},
    };
    for (const auto &test : cases)
    {
      INFO(S(test.Input));
      CHECK(S(CreateUrl(test.Input).ProtocolHostAndPort()) == test.Expected);
    }
  }

  TEST_CASE("URL(SetQuery)", "[URL]")
  {
    Url url = CreateUrl(u8"http://www.webkit.org/?test");
    Url url1 = CreateUrl(u8"http://www.webkit.org/");
    Url url2 = CreateUrl(u8"http://www.webkit.org/?");
    Url url3 = CreateUrl(u8"http://www.webkit.org/?test");
    Url url4 = CreateUrl(u8"http://www.webkit.org/?test1");

    url1.SetQuery(u8"test");
    url2.SetQuery(u8"test");
    url3.SetQuery(u8"test");
    url4.SetQuery(u8"test");

    CHECK(S(url.String()) == S(url1.String()));
    CHECK(S(url.String()) == S(url2.String()));
    CHECK(S(url.String()) == S(url3.String()));
    CHECK(S(url.String()) == S(url4.String()));

    Url urlWithFragmentIdentifier = CreateUrl(u8"http://www.webkit.org/?test%C3%83%C2%A5#newFragment");
    Url urlWithFragmentIdentifier1 = CreateUrl(u8"http://www.webkit.org/#newFragment");
    Url urlWithFragmentIdentifier2 = CreateUrl(u8"http://www.webkit.org/?#newFragment");
    Url urlWithFragmentIdentifier3 = CreateUrl(u8"http://www.webkit.org/?test1#newFragment");

    // Upstream's input is the Latin-1 string "test\xc3\xa5", i.e. U+00C3 U+00A5.
    urlWithFragmentIdentifier1.SetQuery(u8"testÃ¥");
    urlWithFragmentIdentifier2.SetQuery(u8"testÃ¥");
    urlWithFragmentIdentifier3.SetQuery(u8"testÃ¥");

    CHECK(S(urlWithFragmentIdentifier.String()) == S(urlWithFragmentIdentifier1.String()));
    CHECK(S(urlWithFragmentIdentifier.String()) == S(urlWithFragmentIdentifier2.String()));
    CHECK(S(urlWithFragmentIdentifier.String()) == S(urlWithFragmentIdentifier3.String()));

    // A null query removes it.
    Url withQuery = CreateUrl(u8"http://www.webkit.org/?test#fragment");
    withQuery.SetQuery({});
    CHECK(S(withQuery.String()) == "http://www.webkit.org/#fragment");
    CHECK_FALSE(withQuery.HasQuery());
  }

  TEST_CASE("URL(SetFragmentIdentifier)", "[URL]")
  {
    Url url = CreateUrl(u8"http://www.webkit.org/#newFragment%C3%83%C2%A5");
    Url url1 = CreateUrl(u8"http://www.webkit.org/");
    Url url2 = CreateUrl(u8"http://www.webkit.org/#test2");
    Url url3 = CreateUrl(u8"http://www.webkit.org/#");

    url1.SetFragmentIdentifier(u8"newFragmentÃ¥");
    url2.SetFragmentIdentifier(u8"newFragmentÃ¥");
    url3.SetFragmentIdentifier(u8"newFragmentÃ¥");

    CHECK(S(url.String()) == S(url1.String()));
    CHECK(S(url.String()) == S(url2.String()));
    CHECK(S(url.String()) == S(url3.String()));

    Url urlWithQuery = CreateUrl(u8"http://www.webkit.org/?test1#newFragment");
    Url urlWithQuery1 = CreateUrl(u8"http://www.webkit.org/?test1");
    Url urlWithQuery2 = CreateUrl(u8"http://www.webkit.org/?test1#");
    Url urlWithQuery3 = CreateUrl(u8"http://www.webkit.org/?test1#test2");

    urlWithQuery1.SetFragmentIdentifier(u8"newFragment");
    urlWithQuery2.SetFragmentIdentifier(u8"newFragment");
    urlWithQuery3.SetFragmentIdentifier(u8"newFragment");

    CHECK(S(urlWithQuery.String()) == S(urlWithQuery1.String()));
    CHECK(S(urlWithQuery.String()) == S(urlWithQuery2.String()));
    CHECK(S(urlWithQuery.String()) == S(urlWithQuery3.String()));
  }

  TEST_CASE("URL(RemoveQueryAndFragmentIdentifier)", "[URL]")
  {
    Url url = CreateUrl(u8"http://www.webkit.org/");
    Url urls[] {
      CreateUrl(u8"http://www.webkit.org/?"),
      CreateUrl(u8"http://www.webkit.org/?test1"),
      CreateUrl(u8"http://www.webkit.org/?test1#test2"),
      CreateUrl(u8"http://www.webkit.org/#test2"),
      CreateUrl(u8"http://www.webkit.org/#"),
    };

    url.RemoveQueryAndFragmentIdentifier();
    CHECK(S(url.String()) == "http://www.webkit.org/");
    for (auto &other : urls)
    {
      other.RemoveQueryAndFragmentIdentifier();
      CHECK(S(url.String()) == S(other.String()));
    }
  }

  TEST_CASE("URL(EqualIgnoringFragmentIdentifier)", "[URL]")
  {
    struct TestCase
    {
      utf8_stringview Url1;
      utf8_stringview Url2;
      bool Expected;
    };

    TestCase cases[] {
      {u8"http://example.com/", u8"http://example.com/", true},
      {u8"http://example.com/#hash", u8"http://example.com/", true},
      {u8"http://example.com/path", u8"http://example.com/", false},
      {u8"http://example.com/path", u8"http://example.com/path", true},
      {u8"http://example.com/path#hash", u8"http://example.com/path", true},
      {u8"http://example.com/path?query", u8"http://example.com/path", false},
      {u8"http://example.com/path?query#hash", u8"http://example.com/path", false},
      {u8"http://example.com/otherpath", u8"http://example.com/path", false},
      {u8"http://example.com:80/", u8"http://example.com/", true},
      {u8"http://example.com:80/#hash", u8"http://example.com/", true},
      {u8"http://example.com:80/path", u8"http://example.com/", false},
      {u8"http://example.com:80/path#hash", u8"http://example.com/path", true},
      {u8"http://example.com:80/path?query", u8"http://example.com/path", false},
      {u8"http://example.com:80/path?query#hash", u8"http://example.com/path", false},
      {u8"http://example.com:80/otherpath", u8"http://example.com/path", false},
      {u8"http://not-example.com:80/", u8"http://example.com/", false},
      {u8"http://example.com:81/", u8"http://example.com/", false},
      {u8"http://example.com:81/#hash", u8"http://example.com:81/", true},
      {u8"http://example.com:81/path", u8"http://example.com:81", false},
      {u8"http://example.com:81/path#hash", u8"http://example.com:81/path", true},
      {u8"http://example.com:81/path?query", u8"http://example.com:81/path", false},
      {u8"http://example.com:81/path?query#hash", u8"http://example.com:81/path", false},
      {u8"http://example.com:81/otherpath", u8"http://example.com:81/path", false},
      {u8"file:///path/to/file.html", u8"file:///path/to/file.html", true},
      {u8"file:///path/to/file.html#hash", u8"file:///path/to/file.html", true},
      {u8"file:///path/to/file.html?query", u8"file:///path/to/file.html", false},
      {u8"file:///path/to/file.html?query#hash", u8"file:///path/to/file.html", false},
      {u8"file:///path/to/other_file.html", u8"file:///path/to/file.html", false},
      {u8"file:///path/to/other/file.html", u8"file:///path/to/file.html", false},
      {u8"data:text/plain;charset=utf-8;base64,76O/76O/76O/",
       u8"data:text/plain;charset=utf-8;base64,760/760/760/", false},
      {u8"http://example.com", u8"file://example.com", false},
      {u8"http://example.com/#hash", u8"file://example.com", false},
      {u8"http://example.com/?query", u8"file://example.com/", false},
      {u8"http://example.com/?query#hash", u8"file://example.com/", false},
    };

    for (const auto &test : cases)
    {
      INFO(S(test.Url1) << " vs. " << S(test.Url2));
      CHECK(EqualIgnoringFragmentIdentifier(CreateUrl(test.Url1), CreateUrl(test.Url2)) == test.Expected);
    }
  }

  TEST_CASE("URL(ProtocolIsInHTTPFamily)", "[URL]")
  {
    CHECK_FALSE(ProtocolIsInHTTPFamily({}));
    CHECK_FALSE(ProtocolIsInHTTPFamily(u8""));
    CHECK_FALSE(ProtocolIsInHTTPFamily(u8"a"));
    CHECK_FALSE(ProtocolIsInHTTPFamily(u8"ab"));
    CHECK_FALSE(ProtocolIsInHTTPFamily(u8"abc"));
    CHECK_FALSE(ProtocolIsInHTTPFamily(u8"abcd"));
    CHECK_FALSE(ProtocolIsInHTTPFamily(u8"abcde"));
    CHECK_FALSE(ProtocolIsInHTTPFamily(u8"abcdef"));
    CHECK_FALSE(ProtocolIsInHTTPFamily(u8"abcdefg"));
    CHECK(ProtocolIsInHTTPFamily(u8"http:"));
    CHECK_FALSE(ProtocolIsInHTTPFamily(u8"http"));
    CHECK(ProtocolIsInHTTPFamily(u8"https:"));
    CHECK_FALSE(ProtocolIsInHTTPFamily(u8"https"));
    CHECK(ProtocolIsInHTTPFamily(u8"https://!@#$%^&*()"));
  }

  TEST_CASE("URL(HostIsIPAddress)", "[URL]")
  {
    CHECK_FALSE(Url::HostIsIPAddress({}));
    CHECK_FALSE(Url::HostIsIPAddress(u8""));
    CHECK_FALSE(Url::HostIsIPAddress(u8"localhost"));
    CHECK_FALSE(Url::HostIsIPAddress(u8"127.localhost"));
    CHECK_FALSE(Url::HostIsIPAddress(u8"localhost.127"));
    CHECK_FALSE(Url::HostIsIPAddress(u8"127.0.0"));
    CHECK_FALSE(Url::HostIsIPAddress(u8"127.0 .0.1"));
    CHECK_FALSE(Url::HostIsIPAddress(u8" 127.0.0.1"));
    CHECK_FALSE(Url::HostIsIPAddress(u8"127..0.0.1"));
    CHECK_FALSE(Url::HostIsIPAddress(u8"127.0.0."));
    CHECK_FALSE(Url::HostIsIPAddress(u8"256.0.0.1"));
    CHECK_FALSE(Url::HostIsIPAddress(u8"0123:4567:89AB:cdef:3210:7654:ba98"));
    CHECK_FALSE(Url::HostIsIPAddress(u8"012x:4567:89AB:cdef:3210:7654:ba98:FeDc"));
    CHECK_FALSE(Url::HostIsIPAddress(u8"127.0.0.01"));
    CHECK_FALSE(Url::HostIsIPAddress(u8"00123:4567:89AB:cdef:3210:7654:ba98:FeDc"));
    CHECK_FALSE(Url::HostIsIPAddress(u8"0123:4567:89AB:cdef:3210:123.45.67.89"));
    CHECK_FALSE(Url::HostIsIPAddress(u8":::"));
    CHECK_FALSE(Url::HostIsIPAddress(u8"0123::89AB:cdef:3210:7654::FeDc"));
    CHECK_FALSE(Url::HostIsIPAddress(u8"0123:4567:89AB:cdef:3210:7654:ba98:"));
    CHECK_FALSE(Url::HostIsIPAddress(u8"0123:4567:89AB:cdef:3210:7654:ba98:FeDc:"));
    CHECK_FALSE(Url::HostIsIPAddress(u8":4567:89AB:cdef:3210:7654:ba98:FeDc"));
    CHECK_FALSE(Url::HostIsIPAddress(u8":0123:4567:89AB:cdef:3210:7654:ba98:FeDc"));

    CHECK(Url::HostIsIPAddress(u8"127.0.0.1"));
    CHECK(Url::HostIsIPAddress(u8"255.1.10.100"));
    CHECK(Url::HostIsIPAddress(u8"0.0.0.0"));
    CHECK(Url::HostIsIPAddress(u8"::1"));
    CHECK(Url::HostIsIPAddress(u8"::"));
    CHECK(Url::HostIsIPAddress(u8"0123:4567:89AB:cdef:3210:7654:ba98:FeDc"));
    CHECK(Url::HostIsIPAddress(u8"0123:4567:89AB:cdef:3210:7654:ba98::"));
    CHECK(Url::HostIsIPAddress(u8"::4567:89AB:cdef:3210:7654:ba98:FeDc"));
    CHECK(Url::HostIsIPAddress(u8"0123:4567:89AB:cdef:3210:7654:123.45.67.89"));
    CHECK(Url::HostIsIPAddress(u8"::123.45.67.89"));
  }

  TEST_CASE("URL(IsMatchingDomain)", "[URL]")
  {
    Url url = CreateUrl(u8"http://www.webkit.org");

    CHECK(url.IsMatchingDomain({}));
    CHECK(url.IsMatchingDomain(u8""));
    CHECK(url.IsMatchingDomain(u8"org"));
    CHECK(url.IsMatchingDomain(u8"webkit.org"));
    CHECK(url.IsMatchingDomain(u8"www.webkit.org"));

    CHECK_FALSE(url.IsMatchingDomain(u8"rg"));
    CHECK_FALSE(url.IsMatchingDomain(u8".org"));
    CHECK_FALSE(url.IsMatchingDomain(u8"ww.webkit.org"));
    CHECK_FALSE(url.IsMatchingDomain(u8"http://www.webkit.org"));

    url = CreateUrl(u8"file:///www.webkit.org");

    CHECK(url.IsMatchingDomain({}));
    CHECK(url.IsMatchingDomain(u8""));
    CHECK_FALSE(url.IsMatchingDomain(u8"org"));
    CHECK_FALSE(url.IsMatchingDomain(u8"webkit.org"));
    CHECK_FALSE(url.IsMatchingDomain(u8"www.webkit.org"));

    Url emptyUrl;
    CHECK_FALSE(emptyUrl.IsMatchingDomain({}));
    CHECK_FALSE(emptyUrl.IsMatchingDomain(u8""));
  }

  TEST_CASE("URL(DifferingQueryParameters)", "[URL]")
  {
    using Parameters = List<KeyValuePair>;
    const Parameters none {};
    const Parameters key1Val1 {{u8"key1", u8"val1"}};

    CHECK(DifferingQueryParameters(CreateUrl(u8"www.webkit.org/?"),
                                   CreateUrl(u8"http://www.webkit.org/?key1=val1"))
          == key1Val1);
    CHECK(
      DifferingQueryParameters(CreateUrl(u8"http://www.webkit.org/?key1=val1"), CreateUrl(u8"webkit.org/?"))
      == key1Val1);
    CHECK(DifferingQueryParameters(CreateUrl(u8".org/path/?key1=val1"), CreateUrl(u8"/path/?key1=val1"))
          == none);

    Url url = CreateUrl(u8"http://www.webkit.org/?key1=val1");
    CHECK(DifferingQueryParameters(url, url) == none);
    CHECK(DifferingQueryParameters(CreateUrl(u8"http://www.webkit.org/?key1=val1"),
                                   CreateUrl(u8"http://www.webkit.org/?key1=val1"))
          == none);
    CHECK(
      DifferingQueryParameters(CreateUrl(u8"http://www.webkit.org/?"), CreateUrl(u8"http://www.webkit.org/?"))
      == none);
    CHECK(DifferingQueryParameters(CreateUrl(u8"http://www.webkit.org/?"),
                                   CreateUrl(u8"http://www.webkit.org/?key1=val1"))
          == key1Val1);
    CHECK(DifferingQueryParameters(CreateUrl(u8"http://www.webkit.org/?key1=val1"),
                                   CreateUrl(u8"http://www.webkit.org/?"))
          == key1Val1);
    CHECK(DifferingQueryParameters(CreateUrl(u8"http://www.webkit.org/?key1=val1&key2=val2"),
                                   CreateUrl(u8"http://www.webkit.org/?key1=val1&key1=val1"))
          == Parameters {{u8"key1", u8"val1"}, {u8"key2", u8"val2"}});
    CHECK(DifferingQueryParameters(CreateUrl(u8"http://www.webkit.org/?key1=val1"),
                                   CreateUrl(u8"http://www.webkit.org/?key2=val2"))
          == Parameters {{u8"key1", u8"val1"}, {u8"key2", u8"val2"}});
    CHECK(DifferingQueryParameters(CreateUrl(u8"http://www.webkit.org/?key2=val2&key1=val1"),
                                   CreateUrl(u8"http://www.webkit.org/?key3=val3&key1=val1"))
          == Parameters {{u8"key2", u8"val2"}, {u8"key3", u8"val3"}});
    CHECK(DifferingQueryParameters(CreateUrl(u8"http://www.webkit.org/?key1=val1&key2=val2"),
                                   CreateUrl(u8"http://www.webkit.org/?key1&key3=val3"))
          == Parameters {{u8"key1", u8""}, {u8"key1", u8"val1"}, {u8"key2", u8"val2"}, {u8"key3", u8"val3"}});
    CHECK(DifferingQueryParameters(CreateUrl(u8"http://www.webkit.org/?key2=val2&key1=val1&key2=val2"),
                                   CreateUrl(u8"http://www.webkit.org/?key3=val3&key1"))
          == Parameters {{u8"key1", u8""},
                         {u8"key1", u8"val1"},
                         {u8"key2", u8"val2"},
                         {u8"key2", u8"val2"},
                         {u8"key3", u8"val3"}});
    CHECK(DifferingQueryParameters(CreateUrl(u8"http://www.webkit.org/??"),
                                   CreateUrl(u8"http://www.webkit.org/?/?test=test"))
          == Parameters {{u8"/?test", u8"test"}, {u8"?", u8""}});
    CHECK(DifferingQueryParameters(CreateUrl(u8"http://www.webkit.org/?=test"),
                                   CreateUrl(u8"http://www.webkit.org/?=="))
          == Parameters {{u8"", u8"="}, {u8"", u8"test"}});
    CHECK(DifferingQueryParameters(CreateUrl(u8"http://www.webkit.org??"),
                                   CreateUrl(u8"http://www.webkit.org?/?test=test"))
          == Parameters {{u8"/?test", u8"test"}, {u8"?", u8""}});
    CHECK(DifferingQueryParameters(CreateUrl(u8"http://www.webkit.org?=test"),
                                   CreateUrl(u8"http://www.webkit.org?=="))
          == Parameters {{u8"", u8"="}, {u8"", u8"test"}});
    CHECK(DifferingQueryParameters(CreateUrl(u8"http://www.webkit.org?=?"),
                                   CreateUrl(u8"http://www.webkit.org=?"))
          == Parameters {{u8"", u8"?"}});
    CHECK(DifferingQueryParameters(CreateUrl(u8"http://www.webkit.org/?=?"),
                                   CreateUrl(u8"http://www.webkit.org/=?"))
          == Parameters {{u8"", u8"?"}});
  }

  TEST_CASE("URL(IsEqualIgnoringQueryAndFragments)", "[URL]")
  {
    auto equal = [](utf8_stringview a, utf8_stringview b)
    {
      return IsEqualIgnoringQueryAndFragments(CreateUrl(a), CreateUrl(b));
    };

    CHECK_FALSE(equal(u8"www.webkit.org/?", u8"http://www.webkit.org/?key1=val1"));
    CHECK(equal(u8"webkit.org/?=?", u8"webkit.org/?=?"));
    CHECK_FALSE(equal(u8"http://www.webkit.org/", u8"webkit.org/"));
    CHECK_FALSE(equal(u8"webkit.org/?=?", u8"kit.org/?=?"));
    CHECK(equal(u8"http://www.webkit.org/?key1=val1", u8"http://www.webkit.org/?key1=val1"));
    CHECK(equal(u8"http://www.webkit.org/?", u8"http://www.webkit.org/?key1=val1"));
    CHECK(equal(u8"http://www.webkit.org?", u8"http://www.webkit.org/?key1=val1"));
    CHECK_FALSE(equal(u8"http://www.example.org/path?", u8"http://www.webkit.org/?key1=val1"));
    CHECK(equal(u8"http://example.com?a=b", u8"http://example.com/?a=b"));
    CHECK(equal(u8"http://www.webkit.org/?", u8"http://www.webkit.org?"));
    CHECK(equal(u8"http://www.webkit.org/??", u8"http://www.webkit.org/?/?"));
    CHECK(equal(u8"http://www.webkit.org/?=&", u8"http://www.webkit.org/?==&"));
    CHECK(equal(u8"http://www.webkit.org?&?", u8"http://www.webkit.org??&"));
    CHECK_FALSE(equal(u8"http://www.webkit.org?=?", u8"http://www.webkit.org=?"));
    CHECK_FALSE(equal(u8"http://www.webkit.org/?=?", u8"http://www.webkit.org/=?"));
  }

  TEST_CASE("URL(RemoveQueryParameters)", "[URL]")
  {
    const auto url = CreateUrl(u8"http://www.webkit.org/?key=val");
    auto url1 = CreateUrl(u8"http://www.webkit.org/?key=val&key1=val1");
    auto url2 = CreateUrl(u8"http://www.webkit.org/?");
    auto url3 = CreateUrl(u8"http://www.webkit.org/?key=val#fragment");
    auto url4 = CreateUrl(u8"http://www.webkit.org/?key=val&key=val#fragment");
    auto url5 = CreateUrl(u8"http://www.webkit.org/?key&key=#fragment");
    auto url6 = CreateUrl(u8"http://www.webkit.org/#fragment");
    auto url7 = CreateUrl(u8"http://www.webkit.org/?key=val#fragment");
    const auto url8 = CreateUrl(u8"http://www.webkit.org/");
    const auto url9 = CreateUrl(u8"http://www.webkit.org/#fragment");
    const auto url10 = CreateUrl(u8"http://www.webkit.org/?key=val#fragment");
    auto url11 = CreateUrl(u8"http://www.webkit.org/?key=val&key1=val1#fragment");
    auto url12 = CreateUrl(u8"http://www.webkit.org/?key=val&key1=val1#fragment");
    auto url13 = CreateUrl(u8"http://www.webkit.org");
    auto url14 = CreateUrl(u8"http://www.webkit.org/?u+v=x+%20y&key1=foo");
    auto url15 = CreateUrl(u8"http://www.webkit.org/?u+v=x+%20y");

    HashSet<utf8_string> keyRemovalSet1 {u8"key"};
    HashSet<utf8_string> keyRemovalSet2 {u8"key1"};
    HashSet<utf8_string> keyRemovalSet3 {u8"key2"};
    HashSet<utf8_string> keyRemovalSet4 {u8"key", u8"key1"};
    using Removed = List<utf8_string>;

    CHECK(RemoveQueryParameters(url1, keyRemovalSet2) == Removed {u8"key1"});
    CHECK(S(url1.String()) == S(url.String()));

    const auto originalUrl2 = url2;
    CHECK(RemoveQueryParameters(url2, keyRemovalSet1).empty());
    CHECK(S(url2.String()) == S(originalUrl2.String()));

    CHECK(RemoveQueryParameters(url3, keyRemovalSet1) == Removed {u8"key"});
    CHECK(S(url3.String()) == S(url6.String()));

    CHECK(RemoveQueryParameters(url4, keyRemovalSet1) == Removed {u8"key", u8"key"});
    CHECK(S(url4.String()) == S(url6.String()));

    CHECK(RemoveQueryParameters(url5, keyRemovalSet1) == Removed {u8"key", u8"key"});
    CHECK(S(url5.String()) == S(url6.String()));

    CHECK(RemoveQueryParameters(url6, keyRemovalSet1).empty());
    CHECK(S(url6.String()) == S(url9.String()));

    CHECK(RemoveQueryParameters(url7, keyRemovalSet2).empty());
    CHECK(S(url7.String()) == S(url10.String()));

    CHECK(RemoveQueryParameters(url11, keyRemovalSet3).empty());
    CHECK(S(url11.String()) == S(url12.String()));

    CHECK(RemoveQueryParameters(url12, keyRemovalSet4) == Removed {u8"key", u8"key1"});
    CHECK(S(url12.String()) == S(url9.String()));

    CHECK(RemoveQueryParameters(url13, keyRemovalSet1).empty());
    CHECK(S(url13.String()) == S(url8.String()));

    CHECK(RemoveQueryParameters(url14, keyRemovalSet4) == Removed {u8"key1"});
    CHECK(S(url14.String()) == S(url15.String()));
  }

  TEST_CASE("URL(StrippedForUseAsReport)", "[URL]")
  {
    const char *https = "https://example.com/api";
    const char *http = "http://example.com/api";

    CHECK(S(CreateUrl(u8"https://example.com/api").StrippedForUseAsReport()) == https);
    CHECK(S(CreateUrl(u8"https://example.com/api?key=value").StrippedForUseAsReport()) == https);
    CHECK(S(CreateUrl(u8"https://user:password@example.com/api").StrippedForUseAsReport()) == https);
    CHECK(
      S(CreateUrl(u8"https://user:password@example.com/api?key=value&key1=value1").StrippedForUseAsReport())
      == https);
    CHECK(S(CreateUrl(u8"https://example.com/api#fragment").StrippedForUseAsReport()) == https);
    CHECK(S(CreateUrl(u8"https://user:password@example.com/api#fragment").StrippedForUseAsReport()) == https);
    CHECK(S(CreateUrl(u8"https://example.com/api?key=value&key1=value1#fragment").StrippedForUseAsReport())
          == https);
    CHECK(S(CreateUrl(u8"https://user:password@example.com/api?key=value#fragment").StrippedForUseAsReport())
          == https);

    CHECK(S(CreateUrl(u8"http://example.com/api#fragment").StrippedForUseAsReport()) == http);
    CHECK(S(CreateUrl(u8"http://example.com/api?key=value#fragment").StrippedForUseAsReport()) == http);
    CHECK(S(CreateUrl(u8"http://user:password@example.com/api#fragment").StrippedForUseAsReport()) == http);
    CHECK(S(CreateUrl(u8"http://user:password@example.com/api?key=value#fragment").StrippedForUseAsReport())
          == http);
  }

  TEST_CASE("URL(move invalidates)", "[URL]")
  {
    Url url1 {u8"http://www.apple.com"};
    CHECK(url1.IsValid());
    Url url2 {krys::move(url1)};
    CHECK(url2.IsValid());
    CHECK_FALSE(url1.IsValid()); // NOLINT(bugprone-use-after-move,clang-analyzer-cplusplus.Move)

    Url url3 {u8"http://www.webkit.org"};
    url3 = krys::move(url2);
    CHECK(url3.IsValid());
    CHECK_FALSE(url2.IsValid()); // NOLINT(bugprone-use-after-move,clang-analyzer-cplusplus.Move)

    url3 = {};
    CHECK_FALSE(url3.IsValid());
  }

  TEST_CASE("URL(ProtocolIsSecure)", "[URL]")
  {
    CHECK(CreateUrl(u8"https://example.com/api").ProtocolIsSecure());
    CHECK_FALSE(CreateUrl(u8"http://example.com/api").ProtocolIsSecure());
    // Note: FTPS is not considered secure.
    CHECK_FALSE(CreateUrl(u8"ftps://example.com/api").ProtocolIsSecure());
    CHECK_FALSE(CreateUrl(u8"ftp://example.com/api").ProtocolIsSecure());
    CHECK(CreateUrl(u8"wss://example.com/api").ProtocolIsSecure());
    CHECK_FALSE(CreateUrl(u8"ws://example.com/api").ProtocolIsSecure());
    CHECK_FALSE(CreateUrl(u8"asdf:///a/b/c").ProtocolIsSecure());
    CHECK_FALSE(CreateUrl(u8"blob://apple.com").ProtocolIsSecure());
    CHECK_FALSE(CreateUrl(u8"about://example.com/").ProtocolIsSecure());
  }

  TEST_CASE("URL(hash)", "[URL]")
  {
    Url a {u8"http://example.com/"};
    Url b {u8"HTTP://EXAMPLE.COM"};
    CHECK(DefaultHash<Url>::Equal(a, b));
    CHECK(DefaultHash<Url>::Hash(a) == DefaultHash<Url>::Hash(b));

    Url deleted {HashTableDeletedValueTag {}};
    CHECK(HashTraits<Url>::IsDeletedValue(deleted));
    CHECK_FALSE(HashTraits<Url>::IsDeletedValue(a));
  }

  TEST_CASE("URL(setters)", "[URL]")
  {
    Url url {u8"http://example.com/path?query#fragment"};

    CHECK(url.SetProtocol(u8"https"));
    CHECK(S(url.String()) == "https://example.com/path?query#fragment");

    CHECK(url.SetHost(u8"bücher.de"));
    CHECK(S(url.Host()) == "xn--bcher-kva.de");

    url.SetPort(8'443);
    CHECK(S(url.String()) == "https://xn--bcher-kva.de:8443/path?query#fragment");
    url.SetPort(443);
    CHECK(S(url.String()) == "https://xn--bcher-kva.de/path?query#fragment");

    url.SetUser(u8"user name");
    url.SetPassword(u8"p@ss");
    CHECK(S(url.String()) == "https://user%20name:p%40ss@xn--bcher-kva.de/path?query#fragment");
    CHECK(S(url.User()) == "user name");
    CHECK(S(url.Password()) == "p@ss");

    // Percent-encoded bytes that aren't UTF-8 decode to U+FFFD.
    url.SetUser(u8"%FFa%C3%A9");
    CHECK(S(url.User())
          == "\xEF\xBF\xBD"
             "a\xC3\xA9");
    url.RemoveCredentials();
    CHECK_FALSE(url.HasCredentials());

    url.SetPath(u8"a b/c");
    CHECK(S(url.Path()) == "/a%20b/c");

    url.SetHostAndPort(u8"example.org:81");
    CHECK(S(url.HostAndPort()) == "example.org:81");

    url.SetFragmentIdentifier(u8"top");
    CHECK(S(url.FragmentIdentifier()) == "top");

    url.RemoveFragmentIdentifier();
    CHECK_FALSE(url.HasFragmentIdentifier());
  }

  TEST_CASE("URL(string helpers)", "[URL]")
  {
    CHECK(ProtocolIsJavaScript(u8"  java\tscript:alert(1)"));
    CHECK_FALSE(ProtocolIsJavaScript(u8"javascripts:alert(1)"));
    CHECK(ProtocolIs(u8"DATA:text/plain,x", u8"data"));

    CHECK(S(MimeTypeFromDataUrl(u8"data:Text/HTML;charset=utf-8,x")) == "text/html");
    CHECK(S(MimeTypeFromDataUrl(u8"data:,x")) == "text/plain");

    CHECK(DefaultPortForProtocol(u8"https") == 443);
    CHECK_FALSE(DefaultPortForProtocol(u8"file"));
    CHECK(PortAllowed(Url {u8"http://example.com:8080/"}));
    CHECK_FALSE(PortAllowed(Url {u8"http://example.com:25/"}));
    CHECK(PortAllowed(Url {u8"ftp://example.com:21/"}));

    CHECK(AboutBlankUrl().IsAboutBlank());
    CHECK(AboutSrcDocUrl().IsAboutSrcDoc());

    CHECK(S(EncodeWithUrlEscapeSequences(u8"a b/c")) == "a%20b%2Fc");
    CHECK(S(Url::FileUrlWithFileSystemPath(u8"/tmp/a b").String()) == "file:///tmp/a%20b");

    Url base {u8"http://example.com/a/b/c?q#f"};
    CHECK(S(base.TruncatedForUseAsBase().String()) == "http://example.com/a/b/");
    CHECK(S(Url(base, u8"../d").String()) == "http://example.com/a/d");
    CHECK(S(Url(base, {}).String()) == "http://example.com/a/b/c?q");
  }

  TEST_CASE("URL::IsAboutBlank and IsAboutSrcDoc: about:blank with any query, about:srcdoc without one",
            "[URL][html:matches-about:blank][html:matches-about:srcdoc]")
  {
    for (auto blank : {u8"about:blank", u8"about:blank?x", u8"about:blank#f", u8"ABOUT:blank"})
    {
      INFO(reinterpret_cast<const char *>(blank));
      CHECK(Url {blank}.IsAboutBlank());
    }
    for (auto other :
         {u8"about:BLANK", u8"about:blank/", u8"about://host/blank", u8"about:srcdoc", u8"http://blank/"})
    {
      INFO(reinterpret_cast<const char *>(other));
      CHECK_FALSE(Url {other}.IsAboutBlank());
    }

    CHECK(Url {u8"about:srcdoc"}.IsAboutSrcDoc());
    CHECK(Url {u8"about:srcdoc#f"}.IsAboutSrcDoc());
    CHECK_FALSE(Url {u8"about:srcdoc?x"}.IsAboutSrcDoc());
    CHECK_FALSE(Url {u8"about:srcdoc?"}.IsAboutSrcDoc());
    CHECK_FALSE(Url {u8"about:blank"}.IsAboutSrcDoc());
  }
}
