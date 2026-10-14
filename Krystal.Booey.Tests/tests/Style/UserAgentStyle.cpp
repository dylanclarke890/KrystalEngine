#include "Krystal.Booey/Style/UserAgentStyle.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/CSS/ValueTestUtils.hpp"
#include "Krystal.Booey/CSS/Properties/StyleProperties.hpp"
#include "Krystal.Booey/CSS/Rules/KeyframesRule.hpp"
#include "Krystal.Booey/CSS/StyleSheetContents.hpp"
#include "Krystal.Booey/CSS/UserAgentStyleSheets.hpp"
#include "Krystal.Booey/DOM/HTMLDocument.hpp"
#include "Krystal.Booey/Infra/Namespaces.hpp"
#include "Krystal.Booey/Style/Resolver.hpp"
#include "Krystal.Booey/Style/RuleSet.hpp"
#include "Krystal.Core/TypeCast.hpp"

namespace krys::boo::style::tests
{
  using css::tests::ToNarrow;

  namespace
  {
    /// @brief The number of top-level rules in a minified style sheet: blocks, and at-rule statements.
    KRYS_NODISCARD uint32 CountTopLevelRules(css::CSSOMStringView text)
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
          if (!styleRule->Properties().PropertyCount())
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
    void RequireFullyParsed(css::CSSOMStringView text, const css::StyleSheetContents *sheet,
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
      // FIXME: `spelling-error` and `grammar-error` are behind the cssTextDecorationLineErrorValues setting,
      // which is on by default; parser contexts have no settings yet.
      RequireFullyParsed(css::HTMLUserAgentStyleSheet, UserAgentStyle::DefaultStyleSheet,
                         {":root::spelling-error", ":root::grammar-error"});
      RequireFullyParsed(css::QuirksUserAgentStyleSheet, UserAgentStyle::QuirksStyleSheet);
      RequireFullyParsed(css::CounterStylesUserAgentStyleSheet, UserAgentStyle::CounterStylesStyleSheet);
    }

    SECTION("The rule sets hold the sheets' rules")
    {
      REQUIRE(UserAgentStyle::DefaultStyle->RuleCount() > 100u);
      REQUIRE(UserAgentStyle::DefaultQuirksStyle->RuleCount() > 0u);
      REQUIRE(UserAgentStyle::DefaultStyle->TagRules(css::CSSOMStringAtom(u8"div"), true));
      REQUIRE(UserAgentStyle::DefaultQuirksStyle->TagRules(css::CSSOMStringAtom(u8"table"), true));
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
    auto document = CreateRef<dom::HTMLDocument>();

    SECTION("SVG and MathML")
    {
      auto svg =
        document->CreateElementNS(dom::DOMStringAtom(infra::Namespaces::SVG), u8"svg").ReleaseValue();
      UserAgentStyle::EnsureDefaultStyleSheetsForElement(*svg);
      RequireFullyParsed(css::SVGUserAgentStyleSheet, UserAgentStyle::SVGStyleSheet);

      auto version = UserAgentStyle::DefaultStyleVersion;
      UserAgentStyle::EnsureDefaultStyleSheetsForElement(*svg);
      REQUIRE(UserAgentStyle::DefaultStyleVersion == version);

      auto math =
        document->CreateElementNS(dom::DOMStringAtom(infra::Namespaces::MathML), u8"math").ReleaseValue();
      UserAgentStyle::EnsureDefaultStyleSheetsForElement(*math);
      RequireFullyParsed(css::MathMLUserAgentStyleSheet, UserAgentStyle::MathMLStyleSheet);
      RequireFullyParsed(css::MathMLLegacyFontSizeMathUserAgentStyleSheet,
                         UserAgentStyle::MathMLLegacyFontSizeMathStyleSheet);
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

    SECTION("View transitions, with their keyframes")
    {
      auto div = document->CreateElement(u8"div").ReleaseValue();
      UserAgentStyle::EnsureDefaultStyleSheetsForElement(*div);
      RequireFullyParsed(css::ViewTransitionsUserAgentStyleSheet, UserAgentStyle::ViewTransitionsStyleSheet);
      REQUIRE(Resolver::UserAgentKeyframes().contains(css::CSSOMStringAtom(u8"-ua-view-transition-fade-in")));
    }
  }
}
