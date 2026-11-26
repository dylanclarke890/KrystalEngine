#include "Krystal.Web/Css/CSSOM/DOMCSSNamespace.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Web.Tests/Css/ValueTestUtils.hpp"
#include "Krystal.Web/Css/Properties/RegisteredCustomProperty.hpp"
#include "Krystal.Web/Css/Values/SerialisationContext.hpp"
#include "Krystal.Web/Dom/Nodes/HTMLDocument.hpp"
#include "Krystal.Web/Rendering/Style/RenderStyle.hpp"
#include "Krystal.Web/Style/CustomPropertyRegistry.hpp"
#include "Krystal.Web/Style/Scope/Scope.hpp"

namespace krys::web::css::tests
{
  TEST_CASE("CSS.escape", "[CSS][CSSOM][cssom:dom-css-escape]")
  {
    auto escape = [](const char8_t *ident)
    {
      return DOMCSSNamespace::Escape(CSSOMString(ident));
    };

    CHECK(escape(u8"").empty());
    CHECK(escape(u8"abc-_1") == u8"abc-_1");
    CHECK(escape(u8"a b") == u8"a\\ b");
    CHECK(escape(u8"a.b#c") == u8"a\\.b\\#c");
    // A digit can't start an identifier, or follow a hyphen that starts it.
    CHECK(escape(u8"1a") == u8"\\31 a");
    CHECK(escape(u8"-1a") == u8"-\\31 a");
    // A hyphen on its own can't be one either, but two can start one.
    CHECK(escape(u8"-") == u8"\\-");
    CHECK(escape(u8"--a") == u8"--a");
    // Control characters are escaped as code points, and other non-ASCII characters are kept.
    CHECK(escape(u8"a\x7f") == u8"a\\7f ");
    CHECK(escape(u8"é") == u8"é");
  }

  TEST_CASE_METHOD(ValueFixture, "CSS.supports(property, value)",
                   "[CSS][CSSOM][css-conditional:dom-css-supports]")
  {
    auto document = CreateRef<HTMLDocument>();
    auto supports = [&](const char8_t *property, const char8_t *value)
    {
      return DOMCSSNamespace::Supports(*document, property, value);
    };

    CHECK(supports(u8"display", u8"grid"));
    // Property names are ASCII case-insensitive.
    CHECK(supports(u8"DISPLAY", u8"grid"));
    CHECK_FALSE(supports(u8"display", u8"10px"));
    CHECK_FALSE(supports(u8"display", u8""));
    // The value is a value, not a declaration.
    CHECK_FALSE(supports(u8"display", u8"grid !important"));
    CHECK_FALSE(supports(u8"display", u8"grid;"));
    CHECK_FALSE(supports(u8"nonsense", u8"1"));
    // A descriptor isn't a property.
    CHECK_FALSE(supports(u8"src", u8"url(a.ttf)"));
    // A custom property takes any value, including none, but its name is case-sensitive.
    CHECK(supports(u8"--anything", u8"{ a } [ b ]"));
    CHECK(supports(u8"--anything", u8""));
    CHECK_FALSE(supports(u8"--anything", u8"a ) b"));
  }

  TEST_CASE_METHOD(ValueFixture, "CSS.supports(conditionText)",
                   "[CSS][CSSOM][css-conditional:dom-css-supports-conditiontext]")
  {
    auto document = CreateRef<HTMLDocument>();
    auto supports = [&](const char8_t *condition)
    {
      return DOMCSSNamespace::Supports(*document, condition);
    };

    CHECK(supports(u8"(display: grid)"));
    CHECK(supports(u8"(display: grid) and (not (display: foo))"));
    CHECK_FALSE(supports(u8"(display: foo)"));
    // A bare declaration is as if it were in parentheses.
    CHECK(supports(u8"display: grid"));
    CHECK_FALSE(supports(u8"display: foo"));
    CHECK(supports(u8"selector(a > b)"));
    CHECK_FALSE(supports(u8"foo"));
    CHECK_FALSE(supports(u8""));
  }

  TEST_CASE_METHOD(ValueFixture, "CSS.registerProperty()",
                   "[CSS][CSSOM][css-properties-values-api:dom-css-registerproperty]")
  {
    auto document = CreateRef<HTMLDocument>();
    auto &registry = document->StyleScope().GetCustomPropertyRegistry();
    auto exceptionCode = [&](const PropertyDefinition &definition)
    {
      auto result = DOMCSSNamespace::RegisterProperty(*document, definition);
      REQUIRE(result.HasException());
      return result.ReleaseException().Code();
    };

    SECTION("registers the property")
    {
      REQUIRE_FALSE(DOMCSSNamespace::RegisterProperty(*document, {.Name = u8"--size",
                                                                  .Syntax = u8"<length>",
                                                                  .Inherits = true,
                                                                  .InitialValue = CSSOMString(u8"10px")})
                      .HasException());
      const auto *registered = registry.Get(u8"--size");
      REQUIRE(registered);
      CHECK(registered->Inherits);
      REQUIRE(registered->InitialValue);
      CHECK(ToNarrow(registered->InitialValue->PropertyValueSerialisation(
              DefaultSerialisationContext(), RenderStyle::DefaultStyleSingleton()))
            == "10px");

      // The first registration wins.
      CHECK(exceptionCode({.Name = u8"--size", .Syntax = u8"<color>", .InitialValue = CSSOMString(u8"red")})
            == ExceptionCode::InvalidModificationError);
      CHECK(registry.Get(u8"--size")->Inherits);
    }

    SECTION("the universal syntax needs no initial value")
    {
      REQUIRE_FALSE(DOMCSSNamespace::RegisterProperty(*document, {.Name = u8"--any"}).HasException());
      const auto *registered = registry.Get(u8"--any");
      REQUIRE(registered);
      CHECK_FALSE(registered->Inherits);
    }

    SECTION("invalid definitions throw a SyntaxError")
    {
      constexpr auto syntaxError = ExceptionCode::SyntaxError;
      CHECK(exceptionCode({.Name = u8"size", .InitialValue = CSSOMString(u8"1px")}) == syntaxError);
      CHECK(exceptionCode({.Name = u8"--x", .Syntax = u8"<nonsense>", .InitialValue = CSSOMString(u8"1px")})
            == syntaxError);
      CHECK(exceptionCode({.Name = u8"--x", .Syntax = u8"<length>"}) == syntaxError);
      CHECK(exceptionCode({.Name = u8"--x", .Syntax = u8"<length>", .InitialValue = CSSOMString(u8"red")})
            == syntaxError);
      // The initial value must be computationally independent.
      CHECK(exceptionCode({.Name = u8"--x", .Syntax = u8"<length>", .InitialValue = CSSOMString(u8"2em")})
            == syntaxError);
      CHECK_FALSE(registry.Get(u8"--x"));
    }
  }
}
