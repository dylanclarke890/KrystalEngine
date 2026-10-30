#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey/URL/URL.hpp"
#include <algorithm>
#include <cctype>
#include <fstream>
#include <rapidjson/document.h>
#include <sstream>
#include <string>

// Runs the web-platform-tests URL data (url/resources/*.json) against url::URL, mapping each expected field
// to the URL getter the DOM's URL interface uses for it.

namespace krys::boo::url::tests
{
  namespace
  {
    bool IsSurrogateEscape(const std::string &text, size_t i, bool lead)
    {
      if (i + 6 > text.size() || text[i] != '\\' || text[i + 1] != 'u')
      {
        return false;
      }
      for (size_t j = i + 2; j < i + 6; ++j)
      {
        if (!std::isxdigit(static_cast<unsigned char>(text[j])))
        {
          return false;
        }
      }
      unsigned long value = std::stoul(text.substr(i + 2, 4), nullptr, 16);
      return lead ? (value >= 0xD800 && value <= 0xDBFF) : (value >= 0xDC00 && value <= 0xDFFF);
    }

    // The DOM converts strings to USVStrings before parsing, which replaces lone surrogates with U+FFFD; JSON
    // parsers reject them, so do the same to the escapes in the file.
    std::string ReplaceLoneSurrogateEscapes(std::string text)
    {
      for (size_t i = 0; i + 6 <= text.size(); ++i)
      {
        if (text[i] == '\\' && text[i + 1] == '\\')
        {
          ++i;
          continue;
        }
        if (IsSurrogateEscape(text, i, true) && IsSurrogateEscape(text, i + 6, false))
        {
          i += 11;
          continue;
        }
        if (IsSurrogateEscape(text, i, true) || IsSurrogateEscape(text, i, false))
        {
          text.replace(i + 2, 4, "FFFD");
          i += 5;
        }
      }
      return text;
    }

    rapidjson::Document LoadTestData(const char *fileName)
    {
      std::ifstream file(std::string(KRYS_BOOEY_TESTS_DATA_DIR "/url/") + fileName, std::ios::binary);
      REQUIRE(file.is_open());
      std::stringstream contents;
      contents << file.rdbuf();

      rapidjson::Document document;
      document.Parse(ReplaceLoneSurrogateEscapes(contents.str()).c_str());
      REQUIRE_FALSE(document.HasParseError());
      REQUIRE(document.IsArray());
      return document;
    }

    utf8_string U8(const rapidjson::Value &value)
    {
      return utf8_string(reinterpret_cast<const char8 *>(value.GetString()), value.GetStringLength());
    }

    std::string S(utf8_stringview value)
    {
      return std::string(reinterpret_cast<const char *>(value.data()), value.size());
    }

    std::string Expected(const rapidjson::Value &test, const char *member)
    {
      return std::string(test[member].GetString(), test[member].GetStringLength());
    }

    // The DOM URL interface's getters (URLDecomposition upstream).

    std::string Protocol(const URL &url)
    {
      return S(url.Protocol()) + ":";
    }

    std::string Port(const URL &url)
    {
      auto port = url.Port();
      return port ? std::to_string(*port) : std::string();
    }

    std::string Search(const URL &url)
    {
      return url.Query().empty() ? std::string() : S(url.QueryWithLeadingQuestionMark());
    }

    std::string Hash(const URL &url)
    {
      return url.FragmentIdentifier().empty() ? std::string()
                                              : S(url.FragmentIdentifierWithLeadingNumberSign());
    }

    // The DOM URL interface's host and hostname setters.

    void SetHost(URL &url, utf8_stringview value)
    {
      auto copy = url;
      if (value.empty() && !copy.ProtocolIsFile() && copy.HasSpecialScheme())
      {
        return;
      }
      copy.SetHostAndPort(value);
      if (copy.IsValid())
      {
        url = copy;
      }
    }

    void SetHostname(URL &url, utf8_stringview value)
    {
      auto copy = url;
      if (value.empty() && !copy.ProtocolIsFile() && copy.HasSpecialScheme())
      {
        return;
      }
      copy.SetHost(value);
      if (copy.IsValid())
      {
        url = copy;
      }
    }

    utf8_string HTTPSURL(utf8_stringview host)
    {
      utf8_string result {u8"https://"};
      result.append(host);
      result.append(u8"/x");
      return result;
    }
  }

  TEST_CASE("URL(urltestdata.json)", "[URL][WPT]")
  {
    auto document = LoadTestData("urltestdata.json");
    size_t cases = 0;
    for (const auto &test : document.GetArray())
    {
      if (!test.IsObject())
      {
        continue;
      }
      ++cases;

      auto input = U8(test["input"]);
      bool hasBase = test.HasMember("base") && !test["base"].IsNull();
      URL url = hasBase ? URL(URL(U8(test["base"])), input) : URL(input);
      INFO("input: <" << S(input) << "> base: <" << (hasBase ? test["base"].GetString() : "null") << ">");
      INFO("parsed: <" << S(url.String()) << ">");

      if (test.HasMember("failure"))
      {
        CHECK_FALSE(url.IsValid());
        continue;
      }

      REQUIRE(url.IsValid());
      CHECK(S(url.String()) == Expected(test, "href"));
      CHECK(Protocol(url) == Expected(test, "protocol"));
      CHECK(S(url.EncodedUser()) == Expected(test, "username"));
      CHECK(S(url.EncodedPassword()) == Expected(test, "password"));
      CHECK(S(url.HostAndPort()) == Expected(test, "host"));
      CHECK(S(url.Host()) == Expected(test, "hostname"));
      CHECK(Port(url) == Expected(test, "port"));
      CHECK(S(url.Path()) == Expected(test, "pathname"));
      CHECK(Search(url) == Expected(test, "search"));
      CHECK(Hash(url) == Expected(test, "hash"));
    }
    CHECK(cases > 800);
  }

  TEST_CASE("URL(toascii.json)", "[URL][WPT]")
  {
    auto document = LoadTestData("toascii.json");
    for (const auto &test : document.GetArray())
    {
      if (!test.IsObject())
      {
        continue;
      }

      auto input = U8(test["input"]);
      INFO("input: <" << S(input) << ">");
      bool expectFailure = test["output"].IsNull();

      URL url {HTTPSURL(input)};
      if (expectFailure)
      {
        CHECK_FALSE(url.IsValid());
      }
      else
      {
        auto output = Expected(test, "output");
        REQUIRE(url.IsValid());
        CHECK(S(url.HostAndPort()) == output);
        CHECK(S(url.Host()) == output);
        CHECK(S(url.Path()) == "/x");
        CHECK(S(url.String()) == "https://" + output + "/x");
      }

      URL viaHost {u8"https://x/x"};
      SetHost(viaHost, input);
      CHECK(S(viaHost.HostAndPort()) == (expectFailure ? "x" : Expected(test, "output")));

      URL viaHostname {u8"https://x/x"};
      SetHostname(viaHostname, input);
      CHECK(S(viaHostname.Host()) == (expectFailure ? "x" : Expected(test, "output")));
    }
  }

  TEST_CASE("URL(IdnaTestV2.json)", "[URL][WPT]")
  {
    auto document = LoadTestData("IdnaTestV2.json");
    size_t cases = 0;
    for (const auto &test : document.GetArray())
    {
      if (!test.IsObject())
      {
        continue;
      }

      auto input = U8(test["input"]);
      if (input.empty())
      {
        // Can't test the empty string through the URL parser.
        continue;
      }
      ++cases;
      INFO("input: <" << S(input) << ">");

      URL url {HTTPSURL(input)};
      if (test["output"].IsNull())
      {
        // These were generated before Unicode 17 assigned U+32931 and U+32B9A (CJK Extension J), so they
        // expect a V7 (disallowed code point) failure; ICU 78 implements Unicode 17 and accepts them.
        constexpr utf8_stringview assignedSinceUnicode17[] {
          u8"\U0003293120.音.ꡦ1.",
          u8"xn--20-9802c.xn--0w5a.xn--1-eg4e.",
          u8"xn--9-i0j5967eg3qz.ss",
        };
        bool validSinceUnicode17 = std::ranges::find(assignedSinceUnicode17, utf8_stringview {input})
                                   != std::ranges::end(assignedSinceUnicode17);
        CHECK(url.IsValid() == validSinceUnicode17);
        continue;
      }

      auto output = Expected(test, "output");
      REQUIRE(url.IsValid());
      CHECK(S(url.HostAndPort()) == output);
      CHECK(S(url.Host()) == output);
      CHECK(S(url.Path()) == "/x");
      CHECK(S(url.String()) == "https://" + output + "/x");
    }
    CHECK(cases > 2'000);
  }
}
