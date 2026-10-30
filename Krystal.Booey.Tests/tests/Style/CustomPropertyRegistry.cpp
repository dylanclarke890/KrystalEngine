#include "Krystal.Booey/Style/CustomPropertyRegistry.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/CSS/ConsumerTestUtils.hpp"
#include "Krystal.Booey.Tests/CSS/ValueTestUtils.hpp"
#include "Krystal.Booey/CSS/Values/SerialisationContext.hpp"
#include "Krystal.Booey/DOM/HTMLDocument.hpp"
#include "Krystal.Booey/DOM/ShadowRoot.hpp"
#include "Krystal.Booey/HTML/EventLoop.hpp"
#include "Krystal.Booey/HTML/HTMLStyleElement.hpp"
#include "Krystal.Booey/HTML/Parser/HTMLDocumentParser.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Style/CustomProperty.hpp"
#include "Krystal.Booey/Style/Resolver.hpp"
#include "Krystal.Booey/Style/Scope.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesSerialisation.hpp"

namespace krys::boo::style::tests
{
  using css::tests::ToNarrow;

  namespace
  {
    KRYS_NODISCARD Ref<dom::HTMLDocument> Parse(utf8_string &&input) noexcept
    {
      auto document = CreateRef<dom::HTMLDocument>();
      auto parser = CreateRef<html::HTMLDocumentParser>(*document);
      parser->InputStream().Append(krys::move(input), IsEOF(true));
      (void)parser->PumpTokenizer();
      return document;
    }

    void RunTasks() noexcept
    {
      html::EventLoop::Current().RunUntilIdle();
    }

    KRYS_NODISCARD css::CSSOMStringAtom Atom(utf8_stringview name) noexcept
    {
      return css::CSSOMStringAtom {name};
    }

    KRYS_NODISCARD css::CustomPropertySyntax Syntax(utf8_stringview syntax)
    {
      auto parsed = css::CustomPropertySyntax::Parse(syntax);
      REQUIRE(parsed);
      return *parsed;
    }

    KRYS_NODISCARD std::string Serialised(const CustomProperty *property) noexcept
    {
      if (!property)
      {
        return "<null>";
      }
      return ToNarrow(property->PropertyValueSerialisation(css::DefaultSerialisationContext(),
                                                           RenderStyle::DefaultStyleSingleton()));
    }

    /// @brief A registration as an `@property` rule would make it.
    KRYS_NODISCARD css::PropertyRule::Descriptor Descriptor(utf8_stringview name, utf8_stringview syntax,
                                                            bool inherits,
                                                            Maybe<utf8_stringview> initialValue)
    {
      css::PropertyRule::Descriptor descriptor {
        .Name = Atom(name), .Syntax = css::CSSOMString {syntax}, .Inherits = inherits};
      if (initialValue)
      {
        css::tests::Tokens tokens {*initialValue};
        descriptor.InitialValue = css::VariableData::Create(tokens.Tokenizer.Tokens());
      }
      return descriptor;
    }

    KRYS_NODISCARD css::RegisteredCustomProperty Registration(utf8_stringview name, bool inherits)
    {
      return {.Name = Atom(name), .Syntax = Syntax(u8"*"), .Inherits = inherits};
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "CustomPropertyRegistry: registration",
                   "[Style][CustomPropertyRegistry]")
  {
    RunTasks();
    auto document = Parse(u8"<!DOCTYPE html>");
    auto &registry = document->StyleScope().GetCustomPropertyRegistry();

    SECTION("Unregistered properties are inherited")
    {
      CHECK_FALSE(registry.Get(Atom(u8"--x")));
      CHECK(registry.IsInherited(Atom(u8"--x")));
    }

    SECTION("Style sheets: the last registration wins")
    {
      registry.RegisterFromStylesheet(Descriptor(u8"--x", u8"<length>", false, u8"1px"));
      registry.RegisterFromStylesheet(Descriptor(u8"--x", u8"<length>", true, u8"2px"));
      const auto *registered = registry.Get(Atom(u8"--x"));
      REQUIRE(registered);
      CHECK(registered->Inherits);
      CHECK(Serialised(registered->InitialValue.get()) == "2px");
      CHECK_FALSE(registered->InitialValueTokensForViewportUnits);

      registry.ClearRegisteredFromStylesheets();
      CHECK_FALSE(registry.Get(Atom(u8"--x")));
    }

    SECTION("The API: the first registration wins, and wins over style sheets")
    {
      registry.RegisterFromStylesheet(Descriptor(u8"--x", u8"<length>", true, u8"1px"));
      CHECK(registry.RegisterFromAPI(Registration(u8"--x", false)));
      CHECK_FALSE(registry.RegisterFromAPI(Registration(u8"--x", true)));
      CHECK_FALSE(registry.IsInherited(Atom(u8"--x")));

      // Clearing the style sheets' registrations keeps the API's.
      registry.ClearRegisteredFromStylesheets();
      CHECK_FALSE(registry.IsInherited(Atom(u8"--x")));
    }

    SECTION("Invalid initial values aren't registered")
    {
      registry.RegisterFromStylesheet(Descriptor(u8"--relative", u8"<length>", false, u8"1em"));
      registry.RegisterFromStylesheet(Descriptor(u8"--mismatched", u8"<length>", false, u8"red"));
      CHECK_FALSE(registry.Get(Atom(u8"--relative")));
      CHECK_FALSE(registry.Get(Atom(u8"--mismatched")));
    }

    SECTION("The universal syntax doesn't need an initial value")
    {
      registry.RegisterFromStylesheet(Descriptor(u8"--any", u8"*", false, std::nullopt));
      const auto *registered = registry.Get(Atom(u8"--any"));
      REQUIRE(registered);
      CHECK(registered->InitialValue->IsGuaranteedInvalid());
    }

    SECTION("Viewport units")
    {
      registry.RegisterFromStylesheet(Descriptor(u8"--w", u8"<length>", false, u8"10vw"));
      const auto *registered = registry.Get(Atom(u8"--w"));
      REQUIRE(registered);
      CHECK(registered->InitialValueTokensForViewportUnits);
      CHECK(registry.InvalidatePropertiesWithViewportUnits(*document));

      registry.ClearRegisteredFromStylesheets();
      CHECK_FALSE(registry.InvalidatePropertiesWithViewportUnits(*document));
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "CustomPropertyRegistry: initial values",
                   "[Style][CustomPropertyRegistry]")
  {
    RunTasks();
    auto document = Parse(u8"<!DOCTYPE html>");
    auto &registry = document->StyleScope().GetCustomPropertyRegistry();

    SECTION("Parsing")
    {
      css::tests::Tokens absolute {u8"10px"};
      auto parsed = CustomPropertyRegistry::ParseInitialValue(*document, Atom(u8"--x"), Syntax(u8"<length>"),
                                                              absolute.Tokenizer.Tokens());
      REQUIRE(parsed);
      CHECK(Serialised(parsed->first.get()) == "10px");
      CHECK(parsed->second == CustomPropertyRegistry::ViewportUnitDependency::No);

      css::tests::Tokens viewport {u8"10vw"};
      parsed = CustomPropertyRegistry::ParseInitialValue(*document, Atom(u8"--x"), Syntax(u8"<length>"),
                                                         viewport.Tokenizer.Tokens());
      REQUIRE(parsed);
      CHECK(parsed->second == CustomPropertyRegistry::ViewportUnitDependency::Yes);

      css::tests::Tokens relative {u8"2em"};
      parsed = CustomPropertyRegistry::ParseInitialValue(*document, Atom(u8"--x"), Syntax(u8"<length>"),
                                                         relative.Tokenizer.Tokens());
      REQUIRE_FALSE(parsed);
      CHECK(parsed.error() == CustomPropertyRegistry::ParseInitialValueError::NotComputationallyIndependent);

      css::tests::Tokens mismatched {u8"red"};
      parsed = CustomPropertyRegistry::ParseInitialValue(*document, Atom(u8"--x"), Syntax(u8"<length>"),
                                                         mismatched.Tokenizer.Tokens());
      REQUIRE_FALSE(parsed);
      CHECK(parsed.error() == CustomPropertyRegistry::ParseInitialValueError::DidNotParse);
    }

    SECTION("The prototype style holds the initial values")
    {
      registry.RegisterFromStylesheet(Descriptor(u8"--inherited", u8"<length>", true, u8"1px"));
      registry.RegisterFromStylesheet(Descriptor(u8"--not-inherited", u8"<length>", false, u8"2px"));
      registry.RegisterFromStylesheet(Descriptor(u8"--invalid", u8"*", false, std::nullopt));

      const auto &style = registry.InitialValuePrototypeStyle();
      CHECK(Serialised(style.CustomPropertyValue(Atom(u8"--inherited"))) == "1px");
      CHECK(Serialised(style.CustomPropertyValue(Atom(u8"--not-inherited"))) == "2px");
      CHECK(style.InheritedCustomProperties().Size() == 1);
      CHECK(style.NonInheritedCustomProperties().Size() == 1);
      CHECK_FALSE(style.CustomPropertyValue(Atom(u8"--invalid")));

      // Styles start from it.
      auto defaultStyle = Resolver::DefaultStyleForElement(*document, nullptr);
      CHECK(Serialised(defaultStyle->CustomPropertyValue(Atom(u8"--inherited"))) == "1px");

      registry.ClearRegisteredFromStylesheets();
      CHECK_FALSE(registry.InitialValuePrototypeStyle().CustomPropertyValue(Atom(u8"--inherited")));
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "CustomPropertyRegistry: @property rules",
                   "[Style][CustomPropertyRegistry]")
  {
    RunTasks();
    auto document = Parse(u8"<!DOCTYPE html><div id=host></div><style>"
                          u8"@property --x { syntax: '<length>'; inherits: false; initial-value: 5px }"
                          u8"</style>");
    auto &scope = document->StyleScope();
    scope.FlushPendingUpdate();
    std::ignore = scope.GetResolver();

    auto &registry = scope.GetCustomPropertyRegistry();
    const auto *registered = registry.Get(Atom(u8"--x"));
    REQUIRE(registered);
    CHECK_FALSE(registered->Inherits);
    CHECK(Serialised(registered->InitialValue.get()) == "5px");

    SECTION("Clearing the resolver clears them")
    {
      scope.ClearResolver();
      CHECK_FALSE(registry.Get(Atom(u8"--x")));
    }

    SECTION("Shadow trees ignore them")
    {
      auto host = document->GetElementById(u8"host");
      auto shadow = host->AttachShadow({}).ReleaseValue();
      auto style = CreateRef<html::HTMLStyleElement>(*document);
      REQUIRE_FALSE(style->TextContent(u8"@property --y { syntax: '*'; inherits: true }").HasException());
      REQUIRE_FALSE(shadow->AppendChild(*style).HasException());
      shadow->StyleScope().FlushPendingUpdate();
      std::ignore = shadow->StyleScope().GetResolver();

      CHECK_FALSE(registry.Get(Atom(u8"--y")));
      CHECK_FALSE(shadow->StyleScope().GetCustomPropertyRegistry().Get(Atom(u8"--y")));
    }
  }
}
