#include "Krystal.Booey/Style/Rules/UserAgentStyle.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/CSS/ValueTestUtils.hpp"
#include "Krystal.Booey.Tests/Style/ResolverTestUtils.hpp"
#include "Krystal.Booey/CSS/CSSOM/CSSComputedStyleDeclaration.hpp"
#include "Krystal.Booey/CSS/Properties/StyleProperties.hpp"
#include "Krystal.Booey/CSS/Properties/StylePropertiesInlines.hpp"
#include "Krystal.Booey/CSS/Rules/KeyframesRule.hpp"
#include "Krystal.Booey/CSS/StyleSheets/StyleSheetContents.hpp"
#include "Krystal.Booey/CSS/UserAgentStyleSheets.hpp"
#include "Krystal.Booey/DOM/Nodes/HTMLDocument.hpp"
#include "Krystal.Booey/DOM/Nodes/XMLDocument.hpp"
#include "Krystal.Booey/HTML/Window.hpp"
#include "Krystal.Booey/Infra/Namespaces.hpp"
#include "Krystal.Booey/Rendering/RenderTheme.hpp"
#include "Krystal.Booey/Style/Resolver.hpp"
#include "Krystal.Booey/Style/Rules/RuleSet.hpp"
#include "Krystal.Core/TypeCast.hpp"

namespace krys::boo::style::tests
{
  using css::tests::ToNarrow;

  namespace
  {
    /// @brief The number of top-level rules in a minified style sheet: blocks, and at-rule statements.
    KRYS_NODISCARD uint32 CountTopLevelRules(CSSOMStringView text)
    {
      uint32 count = 0;
      uint32 depth = 0;
      for (size_t i = 0; i < text.size(); ++i)
      {
        auto c = text[i];
        if (c == u8'"' || c == u8'\'')
        {
          i = text.find(c, i + 1);
          continue;
        }
        if (c == u8'{')
        {
          if (depth++ == 0)
          {
            ++count;
          }
        }
        else if (c == u8'}')
        {
          --depth;
        }
        else if (c == u8';' && depth == 0)
        {
          ++count;
        }
      }
      return count;
    }

    /// @brief Style rules in @p rules (recursively) with no declarations.
    void CollectEmptyStyleRules(const List<Ref<css::Rule>> &rules, List<std::string> &empty)
    {
      for (auto &rule : rules)
      {
        if (auto *styleRule = DynamicDowncast<css::StyleRule>(*rule))
        {
          if (!styleRule->Properties().size())
          {
            empty.push_back(ToNarrow(styleRule->GetSelectorList().SelectorsText()));
          }
        }
        if (auto *groupRule = DynamicDowncast<css::GroupRule>(*rule))
        {
          CollectEmptyStyleRules(groupRule->ChildRules(), empty);
        }
      }
    }

    /// @brief Checks that every rule of @p text survived parsing into @p sheet, and that only the rules in
    /// @p expectedEmpty lost all their declarations.
    void RequireFullyParsed(CSSOMStringView text, const css::StyleSheetContents *sheet,
                            const List<std::string> &expectedEmpty = {})
    {
      REQUIRE(sheet);
      REQUIRE(sheet->RuleCount() == CountTopLevelRules(text));
      List<std::string> empty;
      CollectEmptyStyleRules(sheet->ChildRules(), empty);
      REQUIRE(empty == expectedEmpty);
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "UserAgentStyle: the default style sheets",
                   "[Style][UserAgentStyle]")
  {
    UserAgentStyle::InitDefaultStyleSheet();

    REQUIRE(UserAgentStyle::DefaultStyle);
    REQUIRE(UserAgentStyle::DefaultQuirksStyle);
    REQUIRE(UserAgentStyle::DefaultPrintStyle);
    REQUIRE(UserAgentStyle::DefaultStyleVersion >= 2u);

    SECTION("Every rule of the sheets parses")
    {
      // The default sheet is the HTML sheet followed by the theme's.
      RequireFullyParsed(CSSOMString(css::HTMLUserAgentStyleSheet)
                           + CSSOMString(RenderTheme::Singleton().ExtraDefaultStyleSheet()),
                         UserAgentStyle::DefaultStyleSheet);
      RequireFullyParsed(css::QuirksUserAgentStyleSheet, UserAgentStyle::QuirksStyleSheet);
      RequireFullyParsed(css::CounterStylesUserAgentStyleSheet, UserAgentStyle::CounterStylesStyleSheet);
    }

    SECTION("The rule sets hold the sheets' rules")
    {
      REQUIRE(UserAgentStyle::DefaultStyle->RuleCount() > 100u);
      REQUIRE(UserAgentStyle::DefaultQuirksStyle->RuleCount() > 0u);
      REQUIRE(UserAgentStyle::DefaultStyle->TagRules(u8"div", true));
      REQUIRE(UserAgentStyle::DefaultQuirksStyle->TagRules(u8"table", true));
      // The user agent sheets don't use IDs.
      REQUIRE(UserAgentStyle::DefaultStyle->Features().IdsInRules.empty());
    }

    SECTION("Rules under non-trivial media queries are kept for the document to evaluate")
    {
      REQUIRE(UserAgentStyle::MediaQueryStyleSheet);
      for (auto &rule : UserAgentStyle::MediaQueryStyleSheet->ChildRules())
      {
        REQUIRE(rule->Type() == css::RuleType::Media);
      }
    }

    SECTION("Initialising again does nothing")
    {
      auto *defaultStyle = UserAgentStyle::DefaultStyle;
      auto version = UserAgentStyle::DefaultStyleVersion;
      UserAgentStyle::InitDefaultStyleSheet();
      REQUIRE(UserAgentStyle::DefaultStyle == defaultStyle);
      REQUIRE(UserAgentStyle::DefaultStyleVersion == version);
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "UserAgentStyle: sheets added for elements",
                   "[Style][UserAgentStyle]")
  {
    UserAgentStyle::InitDefaultStyleSheet();
    auto document = CreateRef<HTMLDocument>();

    SECTION("SVG and MathML")
    {
      auto svg = document->CreateElementNS(infra::namespaces::SVG, u8"svg").ReleaseValue();
      UserAgentStyle::EnsureDefaultStyleSheetsForElement(*svg);
      RequireFullyParsed(css::SVGUserAgentStyleSheet, UserAgentStyle::SVGStyleSheet);

      auto version = UserAgentStyle::DefaultStyleVersion;
      UserAgentStyle::EnsureDefaultStyleSheetsForElement(*svg);
      REQUIRE(UserAgentStyle::DefaultStyleVersion == version);

      auto math = document->CreateElementNS(infra::namespaces::MathML, u8"math").ReleaseValue();
      UserAgentStyle::EnsureDefaultStyleSheetsForElement(*math);
      RequireFullyParsed(css::MathMLUserAgentStyleSheet, UserAgentStyle::MathMLStyleSheet);
      RequireFullyParsed(css::MathMLCoreExtrasUserAgentStyleSheet,
                         UserAgentStyle::MathMLCoreExtrasStyleSheet);
      RequireFullyParsed(css::MathMLFontSizeMathUserAgentStyleSheet,
                         UserAgentStyle::MathMLFontSizeMathStyleSheet);
    }

    SECTION("Form controls and popovers")
    {
      auto input = document->CreateElement(u8"input").ReleaseValue();
      UserAgentStyle::EnsureDefaultStyleSheetsForElement(*input);
      RequireFullyParsed(css::HorizontalFormControlsUserAgentStyleSheet,
                         UserAgentStyle::HorizontalFormControlsStyleSheet);

      auto popover = document->CreateElement(u8"div").ReleaseValue();
      REQUIRE_FALSE(popover->SetAttribute(u8"popover", u8"").HasException());
      UserAgentStyle::EnsureDefaultStyleSheetsForElement(*popover);
      RequireFullyParsed(css::PopoverUserAgentStyleSheet, UserAgentStyle::PopoverStyleSheet);
    }
  }

  TEST_CASE("UserAgentStyle: a form that's a child of a table part isn't rendered",
            "[Style][UserAgentStyle][html:tables-2]")
  {
    ResolverFixture fixture(
      u8"<!DOCTYPE html><table><form id=a></form><tr><td><form id=b></form></td></tr></table>", WithView::No);
    auto &tableForm = fixture.ById(u8"a");
    REQUIRE(tableForm.ParentElement()->LocalName() == u8"table");
    CHECK(fixture.Resolve(tableForm).Style->Display() == DisplayType::None);
    CHECK(fixture.Resolve(fixture.ById(u8"b")).Style->Display() == DisplayType::Block);

    // Only in HTML documents: an XML document's tables can hold forms.
    auto document = CreateRef<XMLDocument>();
    auto table = document->CreateElementNS(infra::namespaces::HTML, u8"table").ReleaseValue();
    auto form = document->CreateElementNS(infra::namespaces::HTML, u8"form").ReleaseValue();
    REQUIRE_FALSE(document->AppendChild(*table).HasException());
    REQUIRE_FALSE(table->AppendChild(*form).HasException());
    auto &resolver = document->StyleScope().GetResolver();
    auto tableStyle = resolver.StyleForElement(*table, {document->InitialContainingBlockStyle()}).Style;
    auto formStyle =
      resolver
        .StyleForElement(*form, {.ParentStyle = tableStyle.get(), .DocumentElementStyle = tableStyle.get()})
        .Style;
    CHECK(formStyle->Display() == DisplayType::Block);
  }

  TEST_CASE("UserAgentStyle: the quirks mode rules",
            "[Style][UserAgentStyle][html:flow-content-3][html:lists][html:tables-2][html:form-controls]["
            "html:images-3]")
  {
    const utf8_string markup =
      u8"<body style='font: italic bold 20px/30px serif; white-space: pre; text-align: center; color: "
      u8"rgb(255, 0, 0)'>"
      u8"<form id=form></form><li id=orphan><ul id=nested><li id=inner></li></ul></li><ul><li "
      u8"id=listed></li></ul>"
      u8"<div style='color: rgb(0, 0, 255)'><table id=table></table></div><input id=text><input id=image "
      u8"type=IMAGE><textarea id=area></textarea>"
      u8"<img id=left align=LEFT><img id=right align=right>";
    ResolverFixture quirks {utf8_string(markup)};
    ResolverFixture standards(u8"<!DOCTYPE html>" + markup);
    REQUIRE(quirks.Document->InQuirksMode());
    REQUIRE_FALSE(standards.Document->InQuirksMode());
    auto computed = [](ResolverFixture &fixture, DOMStringView id, utf8_stringview property)
    {
      auto style = fixture.Document->Window()->GetComputedStyle(fixture.ById(id));
      return ToNarrow(style->GetPropertyValue(CSSOMString(property)));
    };

    CHECK(computed(quirks, u8"form", u8"margin-block-end") == "20px");
    CHECK(computed(standards, u8"form", u8"margin-block-end") == "0px");

    CHECK(computed(quirks, u8"orphan", u8"list-style-position") == "inside");
    CHECK(computed(quirks, u8"nested", u8"list-style-position") == "outside");
    CHECK(computed(quirks, u8"inner", u8"list-style-position") == "outside");
    CHECK(computed(quirks, u8"listed", u8"list-style-position") == "outside");
    CHECK(computed(standards, u8"orphan", u8"list-style-position") == "outside");

    for (utf8_stringview property : {u8"font-weight", u8"font-style", u8"font-size", u8"line-height",
                                     u8"white-space", u8"text-align", u8"color"})
    {
      CAPTURE(ToNarrow(property));
      CHECK(computed(quirks, u8"table", property) != computed(standards, u8"table", property));
    }
    CHECK(computed(quirks, u8"table", u8"font-size") == "16px");
    CHECK(computed(quirks, u8"table", u8"text-align") == "start");
    // Not in the specification: the table takes the body's colour.
    CHECK(computed(quirks, u8"table", u8"color") == "rgb(255, 0, 0)");

    CHECK(computed(quirks, u8"text", u8"box-sizing") == "border-box");
    CHECK(computed(quirks, u8"area", u8"box-sizing") == "border-box");
    CHECK(computed(quirks, u8"image", u8"box-sizing") == "content-box");
    CHECK(computed(standards, u8"text", u8"box-sizing") == "content-box");

    CHECK(computed(quirks, u8"left", u8"margin-right") == "3px");
    CHECK(computed(quirks, u8"right", u8"margin-left") == "3px");
    CHECK(computed(standards, u8"left", u8"margin-right") == "0px");
  }
}
