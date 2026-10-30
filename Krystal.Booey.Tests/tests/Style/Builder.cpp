#include "Krystal.Booey/Style/Builder.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/CSS/ConsumerTestUtils.hpp"
#include "Krystal.Booey.Tests/CSS/RuleTestUtils.hpp"
#include "Krystal.Booey.Tests/Style/BuilderTestUtils.hpp"
#include "Krystal.Booey/CSS/Values/CustomPropertyValue.hpp"
#include "Krystal.Booey/CSS/Values/GlobalKeyword.hpp"
#include "Krystal.Booey/Style/CustomProperty.hpp"
#include "Krystal.Booey/Style/CustomPropertyRegistry.hpp"
#include "Krystal.Booey/Style/MatchResult.hpp"
#include "Krystal.Booey/Style/Scope.hpp"
#include "Krystal.Booey/Style/Values/Primitives/KeywordSerialisation.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesSerialisation.hpp"

namespace krys::boo::style::tests
{
  using css::PropertyId;
  using css::tests::ImmutableDeclarations;
  using css::tests::ToNarrow;

  namespace
  {
    struct BlockOptions
    {
      ScopeOrdinal StyleScopeOrdinal {ScopeOrdinal::Element};
      CascadeLayerPriority CascadeLayerPriority {RuleSet::CascadeLayerPriorityForUnlayered};
    };

    KRYS_NODISCARD MatchedProperties Block(utf8_stringview css, const BlockOptions &options = {})
    {
      return MatchedProperties {.Properties = ImmutableDeclarations(css),
                                .StyleScopeOrdinal = options.StyleScopeOrdinal,
                                .CascadeLayerPriority = options.CascadeLayerPriority};
    }

    /// @brief Builds the fixture's style from matched declarations.
    struct BuilderTestFixture : BuilderFixture
    {
      Ref<MatchResult> Matched {MatchResult::Create()};

      void Build() noexcept
      {
        Builder builder {Style, BuilderContext {.Document = Document, .ParentStyle = &ParentStyle}, *Matched};
        builder.ApplyAllProperties();
      }

      void BuildAuthor(utf8_stringview css) noexcept
      {
        Matched->AuthorDeclarations.push_back(Block(css));
        Build();
      }

      KRYS_NODISCARD std::string Custom(const RenderStyle &style, utf8_stringview name) const noexcept
      {
        const auto *property = style.CustomPropertyValue(css::CSSOMStringAtom {name});
        if (!property)
        {
          return "<none>";
        }
        if (property->IsGuaranteedInvalid())
        {
          return "<invalid>";
        }
        return ToNarrow(property->PropertyValueSerialisation(css::DefaultSerialisationContext(), style));
      }

      KRYS_NODISCARD std::string Custom(utf8_stringview name) const noexcept
      {
        return Custom(Style, name);
      }

      void Register(utf8_stringview name, utf8_stringview syntax, bool inherits, utf8_stringview initialValue)
      {
        css::tests::Tokens tokens {initialValue};
        css::PropertyRule::Descriptor descriptor {.Name = css::CSSOMStringAtom {name},
                                                  .Syntax = css::CSSOMString {syntax},
                                                  .Inherits = inherits,
                                                  .InitialValue =
                                                    css::VariableData::Create(tokens.Tokenizer.Tokens())};
        Document->StyleScope().GetCustomPropertyRegistry().RegisterFromStylesheet(descriptor);
        REQUIRE(Document->StyleScope().GetCustomPropertyRegistry().Get(css::CSSOMStringAtom {name}));
      }
    };
  }

  TEST_CASE_METHOD(BuilderTestFixture, "Builder: the cascade", "[Style][Builder]")
  {
    SECTION("The winning declaration of each property is applied")
    {
      Matched->UserAgentDeclarations.push_back(Block(u8"display: block; opacity: 0.5"));
      Matched->AuthorDeclarations.push_back(Block(u8"opacity: 0.25"));
      Build();
      CHECK(Style.Display() == DisplayType::Block);
      CHECK(Style.GetOpacity() == Opacity {0.25f});
    }

    SECTION("Font properties apply before the properties that depend on them")
    {
      BuildAuthor(u8"width: 2em; font-size: 20px; line-height: 2");
      CHECK(Serialised(Style.Width()) == "40px");
      CHECK(Style.ComputedFontSize() == 20);
    }

    SECTION("Global keywords")
    {
      ParentStyle.SetOpacity(Opacity {0.75f});
      BuildAuthor(u8"opacity: inherit; visibility: hidden");
      CHECK(Style.GetOpacity() == Opacity {0.75f});

      Style = RenderStyle::Clone(ParentStyle);
      Matched = MatchResult::Create();
      Matched->UserAgentDeclarations.push_back(Block(u8"opacity: 0.5"));
      Matched->AuthorDeclarations.push_back(Block(u8"opacity: unset"));
      Build();
      // Opacity isn't inherited, so unset is initial.
      CHECK(Style.GetOpacity() == ComputedStyle::InitialOpacity());
    }

    SECTION("Explicitly inherited non-inherited properties are flagged")
    {
      BuildAuthor(u8"opacity: inherit");
      CHECK(Style.HasExplicitlyInheritedProperties());
    }

    SECTION("Logical properties are resolved against the writing mode")
    {
      BuildAuthor(u8"writing-mode: vertical-rl; margin-inline-start: 4px");
      CHECK(Serialised(Style.MarginTop()) == "4px");
    }

    SECTION("The coordinating list base properties are applied")
    {
      // Author properties that disable native appearance are applied to an element.
      auto div = Document->CreateElement(u8"div");
      REQUIRE_FALSE(div.HasException());
      Reset(div.Value().get());
      BuilderFixture::Build(PropertyId::BackgroundImage, u8"linear-gradient(red, blue), none");
      REQUIRE(Style.GetBackgroundLayers().ComputedLength() == 2);
      CHECK(Style.GetBackgroundLayers().ComputedFirst().HasImage());
      CHECK(Style.GetBackgroundLayers().ComputedFirst().IsImageSet());
      BuilderFixture::Build(PropertyId::MaskImage, u8"linear-gradient(red, blue)");
      CHECK(Style.GetMaskLayers().ComputedFirst().HasImage());
    }

    SECTION("An empty cascade applies nothing")
    {
      Build();
      CHECK(Style.GetOpacity() == ComputedStyle::InitialOpacity());
    }
  }

  TEST_CASE_METHOD(BuilderTestFixture, "Builder: revert and revert-layer", "[Style][Builder]")
  {
    SECTION("revert rolls back to the user agent origin")
    {
      Matched->UserAgentDeclarations.push_back(Block(u8"display: block"));
      Matched->AuthorDeclarations.push_back(Block(u8"display: inline-block"));
      Matched->AuthorDeclarations.push_back(Block(u8"display: revert"));
      Build();
      CHECK(Style.Display() == DisplayType::Block);
    }

    SECTION("revert without a value from an earlier origin is unset")
    {
      BuildAuthor(u8"opacity: 0.5; opacity: revert");
      CHECK(Style.GetOpacity() == ComputedStyle::InitialOpacity());
    }

    SECTION("revert-layer rolls back to the previous layer")
    {
      Matched->AuthorDeclarations.push_back(Block(u8"opacity: 0.25", {.CascadeLayerPriority = 1}));
      Matched->AuthorDeclarations.push_back(Block(u8"opacity: revert-layer", {.CascadeLayerPriority = 2}));
      Build();
      CHECK(Style.GetOpacity() == Opacity {0.25f});
    }

    SECTION("revert on a custom property")
    {
      Matched->UserAgentDeclarations.push_back(Block(u8"--x: ua"));
      Matched->AuthorDeclarations.push_back(Block(u8"--x: author"));
      Matched->AuthorDeclarations.push_back(Block(u8"--x: revert"));
      Build();
      CHECK(Custom(u8"--x") == "ua");
    }
  }

  TEST_CASE_METHOD(BuilderTestFixture, "Builder: var()", "[Style][Builder]")
  {
    SECTION("References are substituted")
    {
      BuildAuthor(u8"--w: 10px; width: var(--w); --nested: var(--w) var(--w)");
      CHECK(Serialised(Style.Width()) == "10px");
      CHECK(Custom(u8"--w") == "10px");
      CHECK(Custom(u8"--nested") == "10px 10px");
    }

    SECTION("Fallbacks")
    {
      BuildAuthor(u8"width: var(--missing, 5px); height: var(--missing, var(--also-missing, 6px))");
      CHECK(Serialised(Style.Width()) == "5px");
      CHECK(Serialised(Style.Height()) == "6px");
    }

    SECTION("A reference without a value or a fallback is invalid at computed-value time")
    {
      Matched->UserAgentDeclarations.push_back(Block(u8"width: 3px"));
      Matched->AuthorDeclarations.push_back(Block(u8"width: var(--missing)"));
      Build();
      // As if unset: width isn't inherited, so it's the initial value.
      CHECK(Serialised(Style.Width()) == "auto");
    }

    SECTION("A substituted value that doesn't parse is invalid at computed-value time")
    {
      BuildAuthor(u8"--c: red; width: var(--c)");
      CHECK(Serialised(Style.Width()) == "auto");
    }

    SECTION("Shorthands")
    {
      BuildAuthor(u8"--m: 1px 2px; margin: var(--m)");
      CHECK(Serialised(Style.MarginTop()) == "1px");
      CHECK(Serialised(Style.MarginLeft()) == "2px");
    }

    SECTION("Inherited custom properties")
    {
      ParentStyle.SetCustomPropertyValue(
        CustomProperty::CreateForGuaranteedInvalid(css::CSSOMStringAtom {u8"--p"}), true);
      Matched->AuthorDeclarations.push_back(Block(u8"--q: inherit"));
      Build();
      CHECK(Custom(u8"--q") == "<invalid>");
    }

    SECTION("Cycles make the properties in them invalid")
    {
      BuildAuthor(u8"--a: var(--b); --b: var(--a); --c: var(--a, x); --d: ok; width: var(--a, 7px)");
      CHECK(Custom(u8"--a") == "<invalid>");
      CHECK(Custom(u8"--b") == "<invalid>");
      // --c isn't in the cycle: its reference to --a falls back.
      CHECK(Custom(u8"--c") == "x");
      CHECK(Custom(u8"--d") == "ok");
      CHECK(Serialised(Style.Width()) == "7px");
    }

    SECTION("Self references are cycles")
    {
      BuildAuthor(u8"--self: var(--self)");
      CHECK(Custom(u8"--self") == "<invalid>");
    }

    SECTION("Keywords in fallbacks of unregistered properties")
    {
      ParentStyle.SetCustomPropertyValue(
        CustomProperty::CreateForVariableData(
          css::CSSOMStringAtom {u8"--k"},
          css::VariableData::Create(css::tests::Tokens {u8"parent"}.Tokenizer.Tokens())),
        true);
      BuildAuthor(u8"--k: var(--missing, inherit)");
      CHECK(Custom(u8"--k") == "parent");
    }
  }

  TEST_CASE_METHOD(BuilderTestFixture, "Builder: registered custom properties", "[Style][Builder]")
  {
    SECTION("Values are computed with the syntax")
    {
      Register(u8"--len", u8"<length>", false, u8"7px");
      BuildAuthor(u8"font-size: 10px; --len: 2em");
      CHECK(Custom(u8"--len") == "20px");
    }

    SECTION("Unset is the initial value of a non-inherited property")
    {
      Register(u8"--len", u8"<length>", false, u8"7px");
      BuildAuthor(u8"--len: unset");
      CHECK(Custom(u8"--len") == "7px");
    }

    SECTION("Values that don't match are unset")
    {
      Register(u8"--len", u8"<length>", true, u8"7px");
      ParentStyle.SetCustomPropertyValue(Ref<const CustomProperty> {Document->StyleScope()
                                                                      .GetCustomPropertyRegistry()
                                                                      .Get(css::CSSOMStringAtom {u8"--len"})
                                                                      ->InitialValue},
                                         true);
      BuildAuthor(u8"--other: red; --len: var(--other)");
      // Inherited, so unset is the parent's value.
      CHECK(Custom(u8"--len") == "7px");
    }

    SECTION("Fallbacks must match the syntax")
    {
      Register(u8"--len", u8"<length>", false, u8"7px");
      BuildAuthor(u8"width: var(--len, red)");
      CHECK(Serialised(Style.Width()) == "auto");
    }

    SECTION("A font-relative value used by font-size is a cycle")
    {
      Register(u8"--f", u8"<length>", false, u8"1px");
      BuildAuthor(u8"--f: 2em; font-size: var(--f)");
      // font-size is invalid at computed-value time, so it inherits.
      CHECK(Style.ComputedFontSize() == ParentStyle.ComputedFontSize());
    }
  }

  TEST_CASE_METHOD(BuilderTestFixture, "Builder: custom properties for container queries", "[Style][Builder]")
  {
    Register(u8"--len", u8"<length>", false, u8"7px");
    Builder builder {Style, BuilderContext {.Document = Document, .ParentStyle = &ParentStyle}, *Matched};

    auto resolve = [&](utf8_stringview name, css::GlobalKeyword keyword)
    {
      auto value = css::CustomPropertyValue::CreateWithGlobalKeyword(css::CSSOMStringAtom {name}, keyword);
      return builder.ResolveCustomPropertyForContainerQueries(*value);
    };

    auto initial = resolve(u8"--len", css::GlobalKeyword::Initial);
    REQUIRE(initial);
    CHECK(Custom(Style, u8"--len") == "<none>");
    CHECK(ToNarrow(initial->PropertyValueSerialisation(css::DefaultSerialisationContext(), Style)) == "7px");

    CHECK(resolve(u8"--unregistered", css::GlobalKeyword::Initial)->IsGuaranteedInvalid());
    CHECK_FALSE(resolve(u8"--len", css::GlobalKeyword::Revert));
  }
}
