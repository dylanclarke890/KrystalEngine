#include "Krystal.Booey/CSS/Values/Primitives/URL.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey/CSS/Parser/ParserContext.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/NumericTypesComputedStyleDependencies.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/NumericTypesSerialisation.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/NumericTypesValueVisitation.hpp"
#include "Krystal.Booey/CSS/Values/SerialisationContext.hpp"
#include <string>

namespace krys::boo::css::tests
{
  namespace
  {
    std::string S(utf8_stringview value)
    {
      return std::string(reinterpret_cast<const char *>(value.data()), value.size());
    }

    ParserContext ContextWithBase(utf8_stringview base)
    {
      ParserContext context;
      context.BaseURL = url::URL {base};
      return context;
    }
  }

  TEST_CASE("CSSURL(CompleteURL)", "[CSS][Primitives][URL]")
  {
    auto context = ContextWithBase(u8"https://example.com/styles/main.css");

    SECTION("resolves against the base URL")
    {
      auto url = CompleteURL(u8"../images/a.png", context);
      REQUIRE(url);
      CHECK(S(url->Specified) == "../images/a.png");
      CHECK(S(url->Resolved.String()) == "https://example.com/images/a.png");
      CHECK(url->Modifiers.LoadedFromOpaqueSource == LoadedFromOpaqueSource::No);
    }

    SECTION("doesn't resolve empty or fragment-only URLs")
    {
      auto empty = CompleteURL(u8"", context);
      REQUIRE(empty);
      CHECK_FALSE(empty->Resolved.IsNull());
      CHECK_FALSE(empty->Resolved.IsValid());

      auto fragment = CompleteURL(u8"#target", context);
      REQUIRE(fragment);
      CHECK(S(fragment->Resolved.String()) == "#target");
      CHECK_FALSE(fragment->Resolved.IsValid());
    }

    SECTION("doesn't resolve against a null or about:blank base")
    {
      auto withoutBase = CompleteURL(u8"a.png", ParserContext {});
      REQUIRE(withoutBase);
      CHECK(withoutBase->Resolved.IsNull());

      auto aboutBlank = CompleteURL(u8"a.png", ContextWithBase(u8"about:blank"));
      REQUIRE(aboutBlank);
      CHECK(aboutBlank->Resolved.IsNull());
    }

    SECTION("carries the context's opaque-source flag")
    {
      context.LoadedFromOpaqueSource = LoadedFromOpaqueSource::Yes;
      auto url = CompleteURL(u8"a.png", context);
      REQUIRE(url);
      CHECK(url->Modifiers.LoadedFromOpaqueSource == LoadedFromOpaqueSource::Yes);
    }
  }

  TEST_CASE("CSSURL(None)", "[CSS][Primitives][URL]")
  {
    CHECK(CSSURL::None().IsNone());

    // `url("")` isn't `none`.
    auto empty = CompleteURL(u8"", ContextWithBase(u8"https://example.com/"));
    REQUIRE(empty);
    CHECK_FALSE(empty->IsNone());
    CHECK_FALSE(CompleteURL(u8"a.png", ParserContext {})->IsNone());
  }

  TEST_CASE("CSSURL(Resolve)", "[CSS][Primitives][URL]")
  {
    auto unresolved = *CompleteURL(u8"https://example.com/a.png", ParserContext {});
    CHECK(unresolved.Resolved.IsNull());
    auto resolved = Resolve(krys::move(unresolved));
    CHECK(S(resolved.Resolved.String()) == "https://example.com/a.png");

    // Already resolved URLs are kept.
    auto relative = *CompleteURL(u8"a.png", ContextWithBase(u8"https://example.com/dir/"));
    CHECK(S(Resolve(krys::move(relative)).Resolved.String()) == "https://example.com/dir/a.png");
  }

  TEST_CASE("CSSURL(MayDependOnBaseURL)", "[CSS][Primitives][URL]")
  {
    auto context = ContextWithBase(u8"https://example.com/");
    CHECK(MayDependOnBaseURL(*CompleteURL(u8"a.png", context)));
    CHECK_FALSE(MayDependOnBaseURL(*CompleteURL(u8"", context)));
    CHECK_FALSE(MayDependOnBaseURL(*CompleteURL(u8"#target", context)));
    CHECK_FALSE(MayDependOnBaseURL(*CompleteURL(u8"data:image/png;base64,AAAA", context)));
  }

  TEST_CASE("CSSURL(Serialise)", "[CSS][Primitives][URL]")
  {
    const auto &context = DefaultSerialisationContext();
    auto url = *CompleteURL(u8"images/a b.png", ContextWithBase(u8"https://example.com/"));

    // The specified URL is serialised, as a string.
    CHECK(S(Serialise(context, url)) == "url(\"images/a b.png\")");
    CHECK(S(Serialise(context, *CompleteURL(u8"say \"hi\"", ParserContext {}))) == "url(\"say \\\"hi\\\"\")");

    url.Modifiers.CrossOrigin = URLCrossOriginFunction {.Parameters = {keywords::UseCredentials {}}};
    url.Modifiers.Integrity = URLIntegrityFunction {.Parameters = u8"sha384-abc"};
    url.Modifiers.ReferrerPolicy = URLReferrerPolicyFunction {.Parameters = {keywords::NoReferrer {}}};
    CHECK(S(Serialise(context, url))
          == "url(\"images/a b.png\" cross-origin(use-credentials) integrity(\"sha384-abc\") "
             "referrer-policy(no-referrer))");
  }

  TEST_CASE("CSSURL(equality, dependencies and visitation)", "[CSS][Primitives][URL]")
  {
    auto context = ContextWithBase(u8"https://example.com/");
    auto a = *CompleteURL(u8"a.png", context);
    CHECK(a == *CompleteURL(u8"a.png", context));
    CHECK_FALSE(a == *CompleteURL(u8"b.png", context));

    auto withModifier = a;
    withModifier.Modifiers.ReferrerPolicy = URLReferrerPolicyFunction {.Parameters = {keywords::Origin {}}};
    CHECK_FALSE(a == withModifier);

    CHECK(CollectComputedStyleDependencies(a).Properties.empty());

    size_t visited = 0;
    Func<IterationStatus(Value &)> visitor = [&](Value &)
    {
      ++visited;
      return IterationStatus::Continue;
    };
    CHECK(VisitValueChildren(visitor, a) == IterationStatus::Continue);
    CHECK(visited == 0);
  }
}
