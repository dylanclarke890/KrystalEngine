#include "Krystal.Booey/CSS/CSSOM/CSSFunctionRule.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/CSS/CSSOMTestUtils.hpp"
#include "Krystal.Booey/CSS/CSSOM/CSSFunctionDeclarations.hpp"
#include "Krystal.Booey/CSS/Descriptors/CSSFunctionDescriptors.hpp"

namespace krys::boo::css::tests
{
  TEST_CASE("CSSFunctionRule", "[CSS][CSSOM][CSSFunctionRule]")
  {
    auto sheet = SheetFromText(u8"@function --double(--x, --y: 2px) { result: calc(var(--x) * 2); }");
    auto &rule = RuleAt<CSSFunctionRule>(*sheet, 0);

    CHECK(TypeOf(rule) == RuleType::Function);
    CHECK(ToNarrow(rule.Name()) == "--double");
    CHECK(ToNarrow(rule.ReturnType()) == "*");

    auto parameters = rule.GetParameters();
    REQUIRE(parameters.size() == 2);
    CHECK(ToNarrow(parameters[0].Name) == "--x");
    CHECK(ToNarrow(parameters[0].Type) == "*");
    CHECK_FALSE(parameters[0].DefaultValue);
    CHECK(ToNarrow(parameters[1].Name) == "--y");
    REQUIRE(parameters[1].DefaultValue);
    CHECK(ToNarrow(*parameters[1].DefaultValue) == "2px");

    CHECK(RuleText(rule) == "@function --double(--x, --y: 2px) { result: calc(var(--x) * 2); }");

    SECTION("declarations")
    {
      REQUIRE(rule.Length() == 1);
      auto &declarations = Downcast<CSSFunctionDeclarations>(*rule.Item(0));
      CHECK(TypeOf(declarations) == RuleType::FunctionDeclarations);
      CHECK(declarations.ParentRule() == &rule);
      CHECK(RuleText(declarations) == "result: calc(var(--x) * 2);");

      auto &style = declarations.Style();
      CHECK(&style == &declarations.Style());
      CHECK(AsDeclaration(style).ParentRule() == &declarations);
      CHECK(ToNarrow(style.Result()) == "calc(var(--x) * 2)");

      // Function descriptors can't be changed yet.
      REQUIRE_FALSE(style.SetResult(u8"1px").HasException());
      CHECK(ToNarrow(style.Result()) == "calc(var(--x) * 2)");
    }
  }

  TEST_CASE("CSSFunctionRule: parameter and return types", "[CSS][CSSOM][CSSFunctionRule]")
  {
    auto sheet =
      SheetFromText(u8"@function --f(--a <length>, --b <length>+: 1px, --c type(<number> | auto#), "
                    u8"--d type(*)) returns type(<color> | none) { result: red; }");
    auto &rule = RuleAt<CSSFunctionRule>(*sheet, 0);

    CHECK(ToNarrow(rule.ReturnType()) == "<color> | none");

    auto parameters = rule.GetParameters();
    REQUIRE(parameters.size() == 4);
    CHECK(ToNarrow(parameters[0].Type) == "<length>");
    CHECK(ToNarrow(parameters[1].Type) == "<length>+");
    CHECK(ToNarrow(parameters[2].Type) == "<number> | auto#");
    CHECK(ToNarrow(parameters[3].Type) == "*");

    // Types of more than one component are wrapped in type(), and universal types are left out.
    CHECK(RuleText(rule)
          == "@function --f(--a <length>, --b <length>+: 1px, --c type(<number> | auto#), --d) "
             "returns type(<color> | none) { result: red; }");
  }
}
