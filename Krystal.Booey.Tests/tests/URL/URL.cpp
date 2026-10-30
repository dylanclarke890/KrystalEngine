#include "Krystal.Booey/URL/URL.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey/URL/URLHash.hpp"
#include <string>

namespace krys::boo::url::tests
{
  namespace
  {
    std::string S(utf8_stringview value)
    {
      return std::string(reinterpret_cast<const char *>(value.data()), value.size());
    }

    URL CreateURL(utf8_stringview string)
    {
      return URL(string);
    }
  }

  TEST_CASE("URL(default)", "[URL]")
  {
    URL url;

    CHECK(url.IsEmpty());
    CHECK(url.IsNull());
    CHECK_FALSE(url.IsValid());
  }

  TEST_CASE("URL(components)", "[URL]")
  {
    URL url {u8"http://username:password@www.example.com:8080/index.html?var=val#fragment"};

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
      CHECK(S(CreateURL(test.Input).ProtocolHostAndPort()) == test.Expected);
    }
  }

  TEST_CASE("URL(SetQuery)", "[URL]")
  {
    URL url = CreateURL(u8"http://www.webkit.org/?test");
    URL url1 = CreateURL(u8"http://www.webkit.org/");
    URL url2 = CreateURL(u8"http://www.webkit.org/?");
    URL url3 = CreateURL(u8"http://www.webkit.org/?test");
    URL url4 = CreateURL(u8"http://www.webkit.org/?test1");

    url1.SetQuery(u8"test");
    url2.SetQuery(u8"test");
    url3.SetQuery(u8"test");
    url4.SetQuery(u8"test");

    CHECK(S(url.String()) == S(url1.String()));
    CHECK(S(url.String()) == S(url2.String()));
    CHECK(S(url.String()) == S(url3.String()));
    CHECK(S(url.String()) == S(url4.String()));

    URL urlWithFragmentIdentifier = CreateURL(u8"http://www.webkit.org/?test%C3%83%C2%A5#newFragment");
    URL urlWithFragmentIdentifier1 = CreateURL(u8"http://www.webkit.org/#newFragment");
    URL urlWithFragmentIdentifier2 = CreateURL(u8"http://www.webkit.org/?#newFragment");
    URL urlWithFragmentIdentifier3 = CreateURL(u8"http://www.webkit.org/?test1#newFragment");

    // Upstream's input is the Latin-1 string "test\xc3\xa5", i.e. U+00C3 U+00A5.
    urlWithFragmentIdentifier1.SetQuery(u8"testÃ¥");
    urlWithFragmentIdentifier2.SetQuery(u8"testÃ¥");
    urlWithFragmentIdentifier3.SetQuery(u8"testÃ¥");

    CHECK(S(urlWithFragmentIdentifier.String()) == S(urlWithFragmentIdentifier1.String()));
    CHECK(S(urlWithFragmentIdentifier.String()) == S(urlWithFragmentIdentifier2.String()));
    CHECK(S(urlWithFragmentIdentifier.String()) == S(urlWithFragmentIdentifier3.String()));

    // A null query removes it.
    URL withQuery = CreateURL(u8"http://www.webkit.org/?test#fragment");
    withQuery.SetQuery({});
    CHECK(S(withQuery.String()) == "http://www.webkit.org/#fragment");
    CHECK_FALSE(withQuery.HasQuery());
  }

  TEST_CASE("URL(SetFragmentIdentifier)", "[URL]")
  {
    URL url = CreateURL(u8"http://www.webkit.org/#newFragment%C3%83%C2%A5");
    URL url1 = CreateURL(u8"http://www.webkit.org/");
    URL url2 = CreateURL(u8"http://www.webkit.org/#test2");
    URL url3 = CreateURL(u8"http://www.webkit.org/#");

    url1.SetFragmentIdentifier(u8"newFragmentÃ¥");
    url2.SetFragmentIdentifier(u8"newFragmentÃ¥");
    url3.SetFragmentIdentifier(u8"newFragmentÃ¥");

    CHECK(S(url.String()) == S(url1.String()));
    CHECK(S(url.String()) == S(url2.String()));
    CHECK(S(url.String()) == S(url3.String()));

    URL urlWithQuery = CreateURL(u8"http://www.webkit.org/?test1#newFragment");
    URL urlWithQuery1 = CreateURL(u8"http://www.webkit.org/?test1");
    URL urlWithQuery2 = CreateURL(u8"http://www.webkit.org/?test1#");
    URL urlWithQuery3 = CreateURL(u8"http://www.webkit.org/?test1#test2");

    urlWithQuery1.SetFragmentIdentifier(u8"newFragment");
    urlWithQuery2.SetFragmentIdentifier(u8"newFragment");
    urlWithQuery3.SetFragmentIdentifier(u8"newFragment");

    CHECK(S(urlWithQuery.String()) == S(urlWithQuery1.String()));
    CHECK(S(urlWithQuery.String()) == S(urlWithQuery2.String()));
    CHECK(S(urlWithQuery.String()) == S(urlWithQuery3.String()));
  }

  TEST_CASE("URL(RemoveQueryAndFragmentIdentifier)", "[URL]")
  {
    URL url = CreateURL(u8"http://www.webkit.org/");
    URL urls[] {
      CreateURL(u8"http://www.webkit.org/?"),
      CreateURL(u8"http://www.webkit.org/?test1"),
      CreateURL(u8"http://www.webkit.org/?test1#test2"),
      CreateURL(u8"http://www.webkit.org/#test2"),
      CreateURL(u8"http://www.webkit.org/#"),
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
      CHECK(EqualIgnoringFragmentIdentifier(CreateURL(test.Url1), CreateURL(test.Url2)) == test.Expected);
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
    CHECK_FALSE(URL::HostIsIPAddress({}));
    CHECK_FALSE(URL::HostIsIPAddress(u8""));
    CHECK_FALSE(URL::HostIsIPAddress(u8"localhost"));
    CHECK_FALSE(URL::HostIsIPAddress(u8"127.localhost"));
    CHECK_FALSE(URL::HostIsIPAddress(u8"localhost.127"));
    CHECK_FALSE(URL::HostIsIPAddress(u8"127.0.0"));
    CHECK_FALSE(URL::HostIsIPAddress(u8"127.0 .0.1"));
    CHECK_FALSE(URL::HostIsIPAddress(u8" 127.0.0.1"));
    CHECK_FALSE(URL::HostIsIPAddress(u8"127..0.0.1"));
    CHECK_FALSE(URL::HostIsIPAddress(u8"127.0.0."));
    CHECK_FALSE(URL::HostIsIPAddress(u8"256.0.0.1"));
    CHECK_FALSE(URL::HostIsIPAddress(u8"0123:4567:89AB:cdef:3210:7654:ba98"));
    CHECK_FALSE(URL::HostIsIPAddress(u8"012x:4567:89AB:cdef:3210:7654:ba98:FeDc"));
    CHECK_FALSE(URL::HostIsIPAddress(u8"127.0.0.01"));
    CHECK_FALSE(URL::HostIsIPAddress(u8"00123:4567:89AB:cdef:3210:7654:ba98:FeDc"));
    CHECK_FALSE(URL::HostIsIPAddress(u8"0123:4567:89AB:cdef:3210:123.45.67.89"));
    CHECK_FALSE(URL::HostIsIPAddress(u8":::"));
    CHECK_FALSE(URL::HostIsIPAddress(u8"0123::89AB:cdef:3210:7654::FeDc"));
    CHECK_FALSE(URL::HostIsIPAddress(u8"0123:4567:89AB:cdef:3210:7654:ba98:"));
    CHECK_FALSE(URL::HostIsIPAddress(u8"0123:4567:89AB:cdef:3210:7654:ba98:FeDc:"));
    CHECK_FALSE(URL::HostIsIPAddress(u8":4567:89AB:cdef:3210:7654:ba98:FeDc"));
    CHECK_FALSE(URL::HostIsIPAddress(u8":0123:4567:89AB:cdef:3210:7654:ba98:FeDc"));

    CHECK(URL::HostIsIPAddress(u8"127.0.0.1"));
    CHECK(URL::HostIsIPAddress(u8"255.1.10.100"));
    CHECK(URL::HostIsIPAddress(u8"0.0.0.0"));
    CHECK(URL::HostIsIPAddress(u8"::1"));
    CHECK(URL::HostIsIPAddress(u8"::"));
    CHECK(URL::HostIsIPAddress(u8"0123:4567:89AB:cdef:3210:7654:ba98:FeDc"));
    CHECK(URL::HostIsIPAddress(u8"0123:4567:89AB:cdef:3210:7654:ba98::"));
    CHECK(URL::HostIsIPAddress(u8"::4567:89AB:cdef:3210:7654:ba98:FeDc"));
    CHECK(URL::HostIsIPAddress(u8"0123:4567:89AB:cdef:3210:7654:123.45.67.89"));
    CHECK(URL::HostIsIPAddress(u8"::123.45.67.89"));
  }

  TEST_CASE("URL(IsMatchingDomain)", "[URL]")
  {
    URL url = CreateURL(u8"http://www.webkit.org");

    CHECK(url.IsMatchingDomain({}));
    CHECK(url.IsMatchingDomain(u8""));
    CHECK(url.IsMatchingDomain(u8"org"));
    CHECK(url.IsMatchingDomain(u8"webkit.org"));
    CHECK(url.IsMatchingDomain(u8"www.webkit.org"));

    CHECK_FALSE(url.IsMatchingDomain(u8"rg"));
    CHECK_FALSE(url.IsMatchingDomain(u8".org"));
    CHECK_FALSE(url.IsMatchingDomain(u8"ww.webkit.org"));
    CHECK_FALSE(url.IsMatchingDomain(u8"http://www.webkit.org"));

    url = CreateURL(u8"file:///www.webkit.org");

    CHECK(url.IsMatchingDomain({}));
    CHECK(url.IsMatchingDomain(u8""));
    CHECK_FALSE(url.IsMatchingDomain(u8"org"));
    CHECK_FALSE(url.IsMatchingDomain(u8"webkit.org"));
    CHECK_FALSE(url.IsMatchingDomain(u8"www.webkit.org"));

    URL emptyURL;
    CHECK_FALSE(emptyURL.IsMatchingDomain({}));
    CHECK_FALSE(emptyURL.IsMatchingDomain(u8""));
  }

  TEST_CASE("URL(DifferingQueryParameters)", "[URL]")
  {
    using Parameters = List<KeyValuePair>;
    const Parameters none {};
    const Parameters key1Val1 {{u8"key1", u8"val1"}};

    CHECK(DifferingQueryParameters(CreateURL(u8"www.webkit.org/?"),
                                   CreateURL(u8"http://www.webkit.org/?key1=val1"))
          == key1Val1);
    CHECK(
      DifferingQueryParameters(CreateURL(u8"http://www.webkit.org/?key1=val1"), CreateURL(u8"webkit.org/?"))
      == key1Val1);
    CHECK(DifferingQueryParameters(CreateURL(u8".org/path/?key1=val1"), CreateURL(u8"/path/?key1=val1"))
          == none);

    URL url = CreateURL(u8"http://www.webkit.org/?key1=val1");
    CHECK(DifferingQueryParameters(url, url) == none);
    CHECK(DifferingQueryParameters(CreateURL(u8"http://www.webkit.org/?key1=val1"),
                                   CreateURL(u8"http://www.webkit.org/?key1=val1"))
          == none);
    CHECK(
      DifferingQueryParameters(CreateURL(u8"http://www.webkit.org/?"), CreateURL(u8"http://www.webkit.org/?"))
      == none);
    CHECK(DifferingQueryParameters(CreateURL(u8"http://www.webkit.org/?"),
                                   CreateURL(u8"http://www.webkit.org/?key1=val1"))
          == key1Val1);
    CHECK(DifferingQueryParameters(CreateURL(u8"http://www.webkit.org/?key1=val1"),
                                   CreateURL(u8"http://www.webkit.org/?"))
          == key1Val1);
    CHECK(DifferingQueryParameters(CreateURL(u8"http://www.webkit.org/?key1=val1&key2=val2"),
                                   CreateURL(u8"http://www.webkit.org/?key1=val1&key1=val1"))
          == Parameters {{u8"key1", u8"val1"}, {u8"key2", u8"val2"}});
    CHECK(DifferingQueryParameters(CreateURL(u8"http://www.webkit.org/?key1=val1"),
                                   CreateURL(u8"http://www.webkit.org/?key2=val2"))
          == Parameters {{u8"key1", u8"val1"}, {u8"key2", u8"val2"}});
    CHECK(DifferingQueryParameters(CreateURL(u8"http://www.webkit.org/?key2=val2&key1=val1"),
                                   CreateURL(u8"http://www.webkit.org/?key3=val3&key1=val1"))
          == Parameters {{u8"key2", u8"val2"}, {u8"key3", u8"val3"}});
    CHECK(DifferingQueryParameters(CreateURL(u8"http://www.webkit.org/?key1=val1&key2=val2"),
                                   CreateURL(u8"http://www.webkit.org/?key1&key3=val3"))
          == Parameters {{u8"key1", u8""}, {u8"key1", u8"val1"}, {u8"key2", u8"val2"}, {u8"key3", u8"val3"}});
    CHECK(DifferingQueryParameters(CreateURL(u8"http://www.webkit.org/?key2=val2&key1=val1&key2=val2"),
                                   CreateURL(u8"http://www.webkit.org/?key3=val3&key1"))
          == Parameters {{u8"key1", u8""},
                         {u8"key1", u8"val1"},
                         {u8"key2", u8"val2"},
                         {u8"key2", u8"val2"},
                         {u8"key3", u8"val3"}});
    CHECK(DifferingQueryParameters(CreateURL(u8"http://www.webkit.org/??"),
                                   CreateURL(u8"http://www.webkit.org/?/?test=test"))
          == Parameters {{u8"/?test", u8"test"}, {u8"?", u8""}});
    CHECK(DifferingQueryParameters(CreateURL(u8"http://www.webkit.org/?=test"),
                                   CreateURL(u8"http://www.webkit.org/?=="))
          == Parameters {{u8"", u8"="}, {u8"", u8"test"}});
    CHECK(DifferingQueryParameters(CreateURL(u8"http://www.webkit.org??"),
                                   CreateURL(u8"http://www.webkit.org?/?test=test"))
          == Parameters {{u8"/?test", u8"test"}, {u8"?", u8""}});
    CHECK(DifferingQueryParameters(CreateURL(u8"http://www.webkit.org?=test"),
                                   CreateURL(u8"http://www.webkit.org?=="))
          == Parameters {{u8"", u8"="}, {u8"", u8"test"}});
    CHECK(DifferingQueryParameters(CreateURL(u8"http://www.webkit.org?=?"),
                                   CreateURL(u8"http://www.webkit.org=?"))
          == Parameters {{u8"", u8"?"}});
    CHECK(DifferingQueryParameters(CreateURL(u8"http://www.webkit.org/?=?"),
                                   CreateURL(u8"http://www.webkit.org/=?"))
          == Parameters {{u8"", u8"?"}});
  }

  TEST_CASE("URL(IsEqualIgnoringQueryAndFragments)", "[URL]")
  {
    auto equal = [](utf8_stringview a, utf8_stringview b)
    {
      return IsEqualIgnoringQueryAndFragments(CreateURL(a), CreateURL(b));
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
    const auto url = CreateURL(u8"http://www.webkit.org/?key=val");
    auto url1 = CreateURL(u8"http://www.webkit.org/?key=val&key1=val1");
    auto url2 = CreateURL(u8"http://www.webkit.org/?");
    auto url3 = CreateURL(u8"http://www.webkit.org/?key=val#fragment");
    auto url4 = CreateURL(u8"http://www.webkit.org/?key=val&key=val#fragment");
    auto url5 = CreateURL(u8"http://www.webkit.org/?key&key=#fragment");
    auto url6 = CreateURL(u8"http://www.webkit.org/#fragment");
    auto url7 = CreateURL(u8"http://www.webkit.org/?key=val#fragment");
    const auto url8 = CreateURL(u8"http://www.webkit.org/");
    const auto url9 = CreateURL(u8"http://www.webkit.org/#fragment");
    const auto url10 = CreateURL(u8"http://www.webkit.org/?key=val#fragment");
    auto url11 = CreateURL(u8"http://www.webkit.org/?key=val&key1=val1#fragment");
    auto url12 = CreateURL(u8"http://www.webkit.org/?key=val&key1=val1#fragment");
    auto url13 = CreateURL(u8"http://www.webkit.org");
    auto url14 = CreateURL(u8"http://www.webkit.org/?u+v=x+%20y&key1=foo");
    auto url15 = CreateURL(u8"http://www.webkit.org/?u+v=x+%20y");

    HashSet<utf8_string> keyRemovalSet1 {u8"key"};
    HashSet<utf8_string> keyRemovalSet2 {u8"key1"};
    HashSet<utf8_string> keyRemovalSet3 {u8"key2"};
    HashSet<utf8_string> keyRemovalSet4 {u8"key", u8"key1"};
    using Removed = List<utf8_string>;

    CHECK(RemoveQueryParameters(url1, keyRemovalSet2) == Removed {u8"key1"});
    CHECK(S(url1.String()) == S(url.String()));

    const auto originalURL2 = url2;
    CHECK(RemoveQueryParameters(url2, keyRemovalSet1).empty());
    CHECK(S(url2.String()) == S(originalURL2.String()));

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

    CHECK(S(CreateURL(u8"https://example.com/api").StrippedForUseAsReport()) == https);
    CHECK(S(CreateURL(u8"https://example.com/api?key=value").StrippedForUseAsReport()) == https);
    CHECK(S(CreateURL(u8"https://user:password@example.com/api").StrippedForUseAsReport()) == https);
    CHECK(
      S(CreateURL(u8"https://user:password@example.com/api?key=value&key1=value1").StrippedForUseAsReport())
      == https);
    CHECK(S(CreateURL(u8"https://example.com/api#fragment").StrippedForUseAsReport()) == https);
    CHECK(S(CreateURL(u8"https://user:password@example.com/api#fragment").StrippedForUseAsReport()) == https);
    CHECK(S(CreateURL(u8"https://example.com/api?key=value&key1=value1#fragment").StrippedForUseAsReport())
          == https);
    CHECK(S(CreateURL(u8"https://user:password@example.com/api?key=value#fragment").StrippedForUseAsReport())
          == https);

    CHECK(S(CreateURL(u8"http://example.com/api#fragment").StrippedForUseAsReport()) == http);
    CHECK(S(CreateURL(u8"http://example.com/api?key=value#fragment").StrippedForUseAsReport()) == http);
    CHECK(S(CreateURL(u8"http://user:password@example.com/api#fragment").StrippedForUseAsReport()) == http);
    CHECK(S(CreateURL(u8"http://user:password@example.com/api?key=value#fragment").StrippedForUseAsReport())
          == http);
  }

  TEST_CASE("URL(move invalidates)", "[URL]")
  {
    URL url1 {u8"http://www.apple.com"};
    CHECK(url1.IsValid());
    URL url2 {krys::move(url1)};
    CHECK(url2.IsValid());
    CHECK_FALSE(url1.IsValid()); // NOLINT(bugprone-use-after-move)

    URL url3 {u8"http://www.webkit.org"};
    url3 = krys::move(url2);
    CHECK(url3.IsValid());
    CHECK_FALSE(url2.IsValid()); // NOLINT(bugprone-use-after-move)

    url3 = {};
    CHECK_FALSE(url3.IsValid());
  }

  TEST_CASE("URL(ProtocolIsSecure)", "[URL]")
  {
    CHECK(CreateURL(u8"https://example.com/api").ProtocolIsSecure());
    CHECK_FALSE(CreateURL(u8"http://example.com/api").ProtocolIsSecure());
    // Note: FTPS is not considered secure.
    CHECK_FALSE(CreateURL(u8"ftps://example.com/api").ProtocolIsSecure());
    CHECK_FALSE(CreateURL(u8"ftp://example.com/api").ProtocolIsSecure());
    CHECK(CreateURL(u8"wss://example.com/api").ProtocolIsSecure());
    CHECK_FALSE(CreateURL(u8"ws://example.com/api").ProtocolIsSecure());
    CHECK_FALSE(CreateURL(u8"asdf:///a/b/c").ProtocolIsSecure());
    CHECK_FALSE(CreateURL(u8"blob://apple.com").ProtocolIsSecure());
    CHECK_FALSE(CreateURL(u8"about://example.com/").ProtocolIsSecure());
  }

  TEST_CASE("URL(hash)", "[URL]")
  {
    URL a {u8"http://example.com/"};
    URL b {u8"HTTP://EXAMPLE.COM"};
    CHECK(DefaultHash<URL>::Equal(a, b));
    CHECK(DefaultHash<URL>::Hash(a) == DefaultHash<URL>::Hash(b));

    URL deleted {HashTableDeletedValueTag {}};
    CHECK(HashTraits<URL>::IsDeletedValue(deleted));
    CHECK_FALSE(HashTraits<URL>::IsDeletedValue(a));
  }

  TEST_CASE("URL(setters)", "[URL]")
  {
    URL url {u8"http://example.com/path?query#fragment"};

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
    url.RemoveCredentials();
    CHECK_FALSE(url.HasCredentials());

    url.SetPath(u8"a b/c");
    CHECK(S(url.Path()) == "/a%20b/c");

    url.SetHostAndPort(u8"example.org:81");
    CHECK(S(url.HostAndPort()) == "example.org:81");

    CHECK(S(url.ConsumeFragmentDirective()).empty());
    url.SetFragmentIdentifier(u8"top:~:text=hello");
    CHECK(S(url.ConsumeFragmentDirective()) == "text=hello");
    CHECK(S(url.FragmentIdentifier()) == "top");

    url.RemoveFragmentIdentifier();
    CHECK_FALSE(url.HasFragmentIdentifier());
  }

  TEST_CASE("URL(string helpers)", "[URL]")
  {
    CHECK(ProtocolIsJavaScript(u8"  java\tscript:alert(1)"));
    CHECK_FALSE(ProtocolIsJavaScript(u8"javascripts:alert(1)"));
    CHECK(ProtocolIs(u8"DATA:text/plain,x", u8"data"));

    CHECK(S(MimeTypeFromDataURL(u8"data:Text/HTML;charset=utf-8,x")) == "text/html");
    CHECK(S(MimeTypeFromDataURL(u8"data:,x")) == "text/plain");

    CHECK(DefaultPortForProtocol(u8"https") == 443);
    CHECK_FALSE(DefaultPortForProtocol(u8"file"));
    CHECK(PortAllowed(URL {u8"http://example.com:8080/"}));
    CHECK_FALSE(PortAllowed(URL {u8"http://example.com:25/"}));
    CHECK(PortAllowed(URL {u8"ftp://example.com:21/"}));

    CHECK(AboutBlankURL().IsAboutBlank());
    CHECK(AboutSrcDocURL().IsAboutSrcDoc());

    CHECK(S(EncodeWithURLEscapeSequences(u8"a b/c")) == "a%20b%2Fc");
    CHECK(S(URL::FileURLWithFileSystemPath(u8"/tmp/a b").String()) == "file:///tmp/a%20b");

    URL base {u8"http://example.com/a/b/c?q#f"};
    CHECK(S(base.TruncatedForUseAsBase().String()) == "http://example.com/a/b/");
    CHECK(S(URL(base, u8"../d").String()) == "http://example.com/a/d");
    CHECK(S(URL(base, {}).String()) == "http://example.com/a/b/c?q");
  }
}
