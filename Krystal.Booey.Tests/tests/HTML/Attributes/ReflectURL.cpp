#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Loader/NavigationTestUtils.hpp"
#include "Krystal.Booey/HTML/Attributes/ReflectAttribute.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLAnchorElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLBaseElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLButtonElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLFormElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLImageElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLInputElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLLinkElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLScriptElement.hpp"

namespace krys::boo::html::tests
{
  using boo::tests::NavigationFixture;
  using boo::tests::Str;

  TEST_CASE("ReflectURL: USVString attributes", "[HTML][Attributes][html:treated-as-a-url]")
  {
    NavigationFixture fixture(u8R"(<img id=img src=" pic.png " srcset="a.png 1x, b.png 2x"><img id=none>
      <img id=bad src="http://[bad"><link id=link href="style.css"><script id=script src="s.js"></script>
      <a id=a href="x" ping="https://p.test/ a.html">)");

    // [ReflectURL] completes the URL against the document's base URL.
    CHECK(Str(fixture.Get<HTMLImageElement>(u8"img").Src()) == "https://example.com/dir/pic.png");
    CHECK(Str(fixture.Get<HTMLLinkElement>(u8"link").Href()) == "https://example.com/dir/style.css");
    CHECK(Str(fixture.Get<HTMLScriptElement>(u8"script").Src()) == "https://example.com/dir/s.js");
    // ...and returns the attribute as it's written when it doesn't parse, or the empty string without one.
    CHECK(Str(fixture.Get<HTMLImageElement>(u8"bad").Src()) == "http://[bad");
    CHECK(Str(fixture.Get<HTMLImageElement>(u8"none").Src()).empty());

    // [Reflect] USVString attributes aren't URLs.
    CHECK(Str(fixture.Get<HTMLImageElement>(u8"img").SrcSet()) == "a.png 1x, b.png 2x");
    CHECK(Str(fixture.Get<HTMLAnchorElement>(u8"a").Ping()) == "https://p.test/ a.html");

    auto &img = fixture.Get<HTMLImageElement>(u8"none");
    img.Src(u8"/root.png");
    CHECK(img.GetAttribute(u8"src") == u8"/root.png");
    CHECK(Str(img.Src()) == "https://example.com/root.png");
  }

  TEST_CASE("ReflectURL: action, formAction and the base element's href",
            "[HTML][Attributes][html:treated-as-a-url]")
  {
    NavigationFixture fixture(u8R"(<base id=base href="https://base.test/b/"><form id=empty action=""></form>
      <form id=noaction></form><form id=f action="go"><button id=b>B</button><input id=i type=submit>
      <button id=fa formaction="other">C</button></form>)");

    // Without an action, the document's URL.
    CHECK(Str(fixture.Get<HTMLFormElement>(u8"empty").Action()) == "https://example.com/dir/page.html");
    CHECK(Str(fixture.Get<HTMLFormElement>(u8"noaction").Action()) == "https://example.com/dir/page.html");
    // Otherwise completed against the base URL, which the base element sets.
    CHECK(Str(fixture.Get<HTMLFormElement>(u8"f").Action()) == "https://base.test/b/go");
    CHECK(Str(fixture.Get<HTMLButtonElement>(u8"b").FormAction()) == "https://example.com/dir/page.html");
    CHECK(Str(fixture.Get<HTMLInputElement>(u8"i").FormAction()) == "https://example.com/dir/page.html");
    CHECK(Str(fixture.Get<HTMLButtonElement>(u8"fa").FormAction()) == "https://base.test/b/other");

    // The base element's href is completed against the document's fallback base URL, not against itself.
    auto &base = fixture.Get<HTMLBaseElement>(u8"base");
    base.Href(u8"relative/");
    CHECK(Str(base.Href()) == "https://example.com/dir/relative/");
  }
}
