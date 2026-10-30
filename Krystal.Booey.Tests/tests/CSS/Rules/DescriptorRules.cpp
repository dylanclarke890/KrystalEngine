#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/CSS/RuleTestUtils.hpp"
#include "Krystal.Booey/CSS/Queries/MediaQueryParser.hpp"
#include "Krystal.Booey/CSS/Rules/FunctionRule.hpp"
#include "Krystal.Booey/CSS/Rules/ImportRule.hpp"
#include "Krystal.Booey/CSS/Rules/PositionTryRule.hpp"
#include "Krystal.Booey/CSS/Rules/ViewTransitionRule.hpp"
#include "Krystal.Booey/CSS/StyleSheetContents.hpp"

namespace krys::boo::css::tests
{
  TEST_CASE_METHOD(ValueFixture, "ViewTransitionRule", "[CSS][Rules][ViewTransitionRule]")
  {
    SECTION("No descriptors")
    {
      auto rule = ViewTransitionRule::Create(MutableStyleProperties::Create(ParserMode::HTMLStandard));
      CHECK(rule->Type() == RuleType::ViewTransition);
      CHECK(rule->IsViewTransitionRule());
      CHECK_FALSE(rule->Navigation());
      CHECK(rule->ComputedNavigation() == ViewTransitionNavigation::None);
      CHECK(rule->Types().empty());
    }

    SECTION("navigation and types")
    {
      auto rule = ViewTransitionRule::Create(
        ParseDescriptors(u8"navigation: auto; types: slide forwards", RuleType::ViewTransition));
      CHECK(rule->Navigation() == ViewTransitionNavigation::Auto);
      CHECK(rule->ComputedNavigation() == ViewTransitionNavigation::Auto);
      REQUIRE(rule->Types().size() == 2);
      CHECK(rule->Types()[0] == CSSOMStringAtom(u8"slide"));
      CHECK(rule->Types()[1] == CSSOMStringAtom(u8"forwards"));

      auto copied = static_cast<const Rule &>(*rule).Copy();
      auto &copy = Downcast<ViewTransitionRule>(*copied);
      CHECK(copy.Navigation() == ViewTransitionNavigation::Auto);
      CHECK(copy.Types().size() == 2);
    }

    SECTION("navigation: none and types: none")
    {
      auto rule = ViewTransitionRule::Create(
        ParseDescriptors(u8"navigation: none; types: none", RuleType::ViewTransition));
      CHECK(rule->Navigation() == ViewTransitionNavigation::None);
      CHECK(rule->Types().empty());
    }
  }

  TEST_CASE_METHOD(ValueFixture, "PositionTryRule", "[CSS][Rules][PositionTryRule]")
  {
    auto rule = PositionTryRule::Create(CSSOMStringAtom(u8"--flip"), ImmutableDeclarations(u8"top: 1px"));
    CHECK(rule->Type() == RuleType::PositionTry);
    CHECK(rule->IsPositionTryRule());
    CHECK(rule->Name() == CSSOMStringAtom(u8"--flip"));
    CHECK(AsText(rule->Properties()) == "top: 1px;");
    CHECK(rule->ProtectedProperties().get() == &rule->Properties());
    CHECK_FALSE(rule->Properties().IsMutable());

    auto copy = rule->Copy();
    CHECK(&copy->Properties() != &rule->Properties());
    CHECK(copy->Properties().IsMutable());

    auto mutableProperties = rule->ProtectedMutableProperties();
    CHECK(mutableProperties.get() == &rule->MutableProperties());
    CHECK(rule->Properties().IsMutable());
  }

  TEST_CASE_METHOD(ValueFixture, "FunctionRule", "[CSS][Rules][FunctionRule]")
  {
    auto body = FunctionDeclarationsRule::Create(ParseDeclarations(u8"--x: 1px")->ImmutableCopyIfNeeded());
    CHECK(body->Type() == RuleType::FunctionDeclarations);
    CHECK(Is<FunctionDeclarationsRule>(static_cast<Rule &>(*body)));
    CHECK_FALSE(body->Properties().IsMutable());
    CHECK(body->MutableProperties().IsMutable());

    List<FunctionRule::Parameter> parameters;
    parameters.push_back({CSSOMStringAtom(u8"--a"), CustomPropertySyntax::Universal(), nullptr});
    auto rule = FunctionRule::Create(CSSOMStringAtom(u8"--f"), krys::move(parameters),
                                     CustomPropertySyntax::Universal(), {body});
    CHECK(rule->Type() == RuleType::Function);
    CHECK(rule->IsGroupRule());
    CHECK(Is<FunctionRule>(static_cast<Rule &>(*rule)));
    CHECK(rule->Name() == CSSOMStringAtom(u8"--f"));
    REQUIRE(rule->Parameters().size() == 1);
    CHECK(rule->Parameters()[0].Name == CSSOMStringAtom(u8"--a"));
    CHECK(rule->ReturnType().IsUniversal());

    auto copy = rule->Copy();
    REQUIRE(copy->ChildRules().size() == 1);
    CHECK(copy->ChildRules()[0].get() != body.get());
    CHECK(copy->Parameters().size() == 1);
  }

  TEST_CASE_METHOD(ValueFixture, "ImportRule", "[CSS][Rules][ImportRule]")
  {
    auto rule =
      ImportRule::Create(u8"a.css", mq::MediaQueryParser::Parse(u8"print", StrictParserContext()),
                         CascadeLayerName {CSSOMStringAtom(u8"base")}, {u8"(display: grid)", false});
    CHECK(rule->Type() == RuleType::Import);
    CHECK(rule->IsImportRule());
    CHECK(rule->Href() == u8"a.css");
    CHECK(rule->MediaQueries().size() == 1);
    REQUIRE(rule->GetCascadeLayerName());
    CHECK(*rule->GetCascadeLayerName() == CascadeLayerName {CSSOMStringAtom(u8"base")});
    CHECK(rule->SupportsText() == u8"(display: grid)");
    CHECK_FALSE(rule->SupportsMatches());
    CHECK_FALSE(rule->IsLoading());
    CHECK(rule->StyleSheet() == nullptr);
    CHECK(rule->ParentStyleSheet() == nullptr);

    rule->SetMediaQueries({});
    CHECK(rule->MediaQueries().empty());

    // Not loaded, so cancelling does nothing.
    rule->CancelLoad();

    auto sheet = StyleSheetContents::Create();
    rule->SetParentStyleSheet(sheet.get());
    CHECK(rule->ParentStyleSheet() == sheet.get());
    rule->ClearParentStyleSheet();
    CHECK(rule->ParentStyleSheet() == nullptr);

    auto defaults = ImportRule::Create(u8"b.css", {}, {}, {});
    CHECK_FALSE(defaults->GetCascadeLayerName());
    CHECK(defaults->SupportsMatches());
    CHECK(defaults->SupportsText().empty());
  }
}
