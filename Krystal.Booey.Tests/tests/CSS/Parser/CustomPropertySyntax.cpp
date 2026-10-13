#include "Krystal.Booey/CSS/Parser/CustomPropertySyntax.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey/CSS/Parser/InputStream.hpp"
#include "Krystal.Booey/CSS/Parser/Tokenizer.hpp"
#include "Krystal.Booey/CSS/Parser/TokenRange.hpp"
#include "Krystal.Core/Types/Pair.hpp"

namespace krys::boo::css::tests
{
  namespace
  {
    using Type = CustomPropertySyntax::Type;
    using Multiplier = CustomPropertySyntax::Multiplier;

    Maybe<CustomPropertySyntax::Component> Single(CSSOMStringView syntax)
    {
      auto parsed = CustomPropertySyntax::Parse(syntax);
      if (!parsed || parsed->Definition.size() != 1uz)
      {
        return {};
      }
      return parsed->Definition[0];
    }

    /// @brief Runs ConsumeType on `css` and returns the result and the remaining tokens' serialisation.
    Pair<Maybe<CustomPropertySyntax>, CSSOMString> ConsumeType(utf8_stringview css)
    {
      InputStream input {CSSOMString(css)};
      Tokenizer tokenizer {input};
      tokenizer.PumpTokenizer();
      auto tokens = tokenizer.Tokens();
      auto type = CustomPropertySyntax::ConsumeType(tokens);
      return {krys::move(type), tokens.Serialise(TokenSerialisationMode::Normal)};
    }
  }

  TEST_CASE("CustomPropertySyntax::Parse", "[CSS][Parser][CustomPropertySyntax]")
  {
    SECTION("Universal")
    {
      for (auto syntax : {u8"*", u8"  *  ", u8"\t*\n"})
      {
        auto parsed = CustomPropertySyntax::Parse(syntax);
        REQUIRE(parsed);
        CHECK(parsed->IsUniversal());
      }
      CHECK_FALSE(CustomPropertySyntax::Parse(u8"* *"));
      CHECK_FALSE(CustomPropertySyntax::Parse(u8"*+"));
      CHECK(CustomPropertySyntax::Universal().IsUniversal());
    }

    SECTION("Data type names")
    {
      struct Case
      {
        const char8 *Syntax;
        Type Expected;
      };

      for (auto [syntax, expected] : {
             Case {u8"<angle>", Type::Angle},
             Case {u8"<color>", Type::Color},
             Case {u8"<custom-ident>", Type::CustomIdent},
             Case {u8"<image>", Type::Image},
             Case {u8"<integer>", Type::Integer},
             Case {u8"<length>", Type::Length},
             Case {u8"<length-percentage>", Type::LengthPercentage},
             Case {u8"<number>", Type::Number},
             Case {u8"<percentage>", Type::Percentage},
             Case {u8"<resolution>", Type::Resolution},
             Case {u8"<string>", Type::String},
             Case {u8"<time>", Type::Time},
             Case {u8"<transform-function>", Type::TransformFunction},
             Case {u8"<transform-list>", Type::TransformList},
             Case {u8"<url>", Type::URL},
             Case {u8"<foo>", Type::Unknown},
             // Type names are case-sensitive.
             Case {u8"<Length>", Type::Unknown},
           })
      {
        auto component = Single(syntax);
        REQUIRE(component);
        CHECK(component->Type == expected);
        CHECK(component->Multiplier == Multiplier::Single);
        CHECK(component->Ident == CSSOMStringAtom::Null());
      }
    }

    SECTION("Multipliers")
    {
      auto spaceList = Single(u8"<length>+");
      REQUIRE(spaceList);
      CHECK(spaceList->Multiplier == Multiplier::SpaceList);

      auto commaList = Single(u8"<length># ");
      REQUIRE(commaList);
      CHECK(commaList->Multiplier == Multiplier::CommaList);

      // <transform-list> is pre-multiplied.
      auto transformList = Single(u8"<transform-list>+");
      REQUIRE(transformList);
      CHECK(transformList->Type == Type::Unknown);

      CHECK_FALSE(CustomPropertySyntax::Parse(u8"<length>++"));
      CHECK_FALSE(CustomPropertySyntax::Parse(u8"<length> +"));
    }

    SECTION("Idents")
    {
      auto component = Single(u8"foo");
      REQUIRE(component);
      CHECK(component->Type == Type::CustomIdent);
      CHECK(component->Ident == CSSOMStringAtom(u8"foo"));

      auto list = Single(u8"foo#");
      REQUIRE(list);
      CHECK(list->Ident == CSSOMStringAtom(u8"foo"));
      CHECK(list->Multiplier == Multiplier::CommaList);

      // Not valid custom identifiers.
      CHECK_FALSE(CustomPropertySyntax::Parse(u8"initial"));
      CHECK_FALSE(CustomPropertySyntax::Parse(u8"default"));
      CHECK_FALSE(CustomPropertySyntax::Parse(u8"foo bar"));
      CHECK_FALSE(CustomPropertySyntax::Parse(u8"1px"));
      CHECK_FALSE(CustomPropertySyntax::Parse(u8"'foo'"));
    }

    SECTION("Alternatives")
    {
      auto parsed = CustomPropertySyntax::Parse(u8" <length> | auto | <color>#");
      REQUIRE(parsed);
      REQUIRE(parsed->Definition.size() == 3uz);
      CHECK(parsed->Definition[0].Type == Type::Length);
      CHECK(parsed->Definition[1].Type == Type::CustomIdent);
      CHECK(parsed->Definition[1].Ident == CSSOMStringAtom(u8"auto"));
      CHECK(parsed->Definition[2].Type == Type::Color);
      CHECK(parsed->Definition[2].Multiplier == Multiplier::CommaList);
      CHECK_FALSE(parsed->IsUniversal());
      CHECK_FALSE(parsed->ContainsUnknownType());

      // A trailing bar is allowed.
      auto trailing = CustomPropertySyntax::Parse(u8"<length> |");
      REQUIRE(trailing);
      CHECK(trailing->Definition.size() == 1uz);

      auto unknown = CustomPropertySyntax::Parse(u8"<length> | <foo>");
      REQUIRE(unknown);
      CHECK(unknown->ContainsUnknownType());
    }

    SECTION("Invalid")
    {
      for (auto syntax :
           {u8"", u8"   ", u8"<>", u8"<length", u8"<length> extra", u8"<length> | | <color>", u8"| <length>"})
      {
        CAPTURE(reinterpret_cast<const char *>(syntax));
        CHECK_FALSE(CustomPropertySyntax::Parse(syntax));
      }
    }
  }

  TEST_CASE("CustomPropertySyntax::ConsumeType", "[CSS][Parser][CustomPropertySyntax]")
  {
    SECTION("Syntax component")
    {
      auto [type, rest] = ConsumeType(u8"<length>, 1px");
      REQUIRE(type);
      REQUIRE(type->Definition.size() == 1uz);
      CHECK(type->Definition[0].Type == Type::Length);
      CHECK(rest == u8", 1px");
    }

    SECTION("Stops at a colon or whitespace")
    {
      auto [colonType, colonRest] = ConsumeType(u8"<color>#: red");
      REQUIRE(colonType);
      CHECK(colonType->Definition[0].Multiplier == Multiplier::CommaList);
      CHECK(colonRest == u8": red");

      auto [spaceType, spaceRest] = ConsumeType(u8"foo bar");
      REQUIRE(spaceType);
      CHECK(spaceType->Definition[0].Ident == CSSOMStringAtom(u8"foo"));
      CHECK(spaceRest == u8"bar");
    }

    SECTION("type()")
    {
      auto [type, rest] = ConsumeType(u8"TYPE(<length> | <percentage>) x");
      REQUIRE(type);
      REQUIRE(type->Definition.size() == 2uz);
      CHECK(type->Definition[1].Type == Type::Percentage);
      CHECK(rest == u8"x");

      auto [universal, universalRest] = ConsumeType(u8"type(*)");
      REQUIRE(universal);
      CHECK(universal->IsUniversal());
    }

    SECTION("Invalid")
    {
      CHECK_FALSE(ConsumeType(u8"<foo>").first);
      CHECK_FALSE(ConsumeType(u8"type(<foo>)").first);
      CHECK_FALSE(ConsumeType(u8"other(<length>)").first);
      // Only one component outside type().
      CHECK_FALSE(ConsumeType(u8"<length>|<color>").first);
    }
  }
}
