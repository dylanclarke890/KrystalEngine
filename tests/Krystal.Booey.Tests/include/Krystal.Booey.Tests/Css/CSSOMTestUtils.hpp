#pragma once

#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Css/StylePropertiesTestUtils.hpp"
#include "Krystal.Booey/Css/CSSOM/CSSRule.hpp"
#include "Krystal.Booey/Css/CSSOM/CSSRuleList.hpp"
#include "Krystal.Booey/Css/CSSOM/CSSStyleDeclaration.hpp"
#include "Krystal.Booey/Css/CSSOM/CSSStyleSheet.hpp"
#include "Krystal.Booey/Css/Parser/Parser.hpp"
#include "Krystal.Booey/Css/StyleSheets/StyleSheetContents.hpp"
#include "Krystal.Booey/Css/Values/SerialisationContext.hpp"
#include "Krystal.Booey/Dom/Types/ExceptionOr.hpp"

namespace krys::boo::css::tests
{
  /// @brief A parser context with the settings-gated rules and properties the CSSOM tests use enabled.
  KRYS_NODISCARD inline ParserContext CSSOMTestContext()
  {
    auto context = DeclarationTestContext();
    return context;
  }

  /// @brief A style sheet (not constructed by script) holding `css`, parsed with `CSSOMTestContext`.
  KRYS_NODISCARD inline Ref<CSSStyleSheet> SheetFromText(utf8_stringview css)
  {
    auto context = CSSOMTestContext();
    auto contents = StyleSheetContents::Create(context);
    Parser::ParseStyleSheet(css, context, *contents);
    contents->CheckLoaded();
    return CSSStyleSheet::Create(krys::move(contents));
  }

  /// @brief The rule type of `rule` (wrappers keep `StyleRuleType` private).
  KRYS_NODISCARD inline RuleType TypeOf(const CSSRule &rule)
  {
    return rule.StyleRuleType();
  }

  /// @brief `declaration` through its base, whose members the descriptor classes keep protected.
  KRYS_NODISCARD inline CSSStyleDeclaration &AsDeclaration(CSSStyleDeclaration &declaration)
  {
    return declaration;
  }

  /// @brief The `cssText` of `rule`, as a narrow string.
  KRYS_NODISCARD inline std::string RuleText(const CSSRule &rule)
  {
    return ToNarrow(rule.CssText());
  }

  /// @brief The `cssText` of `sheet`, as a narrow string.
  KRYS_NODISCARD inline std::string SheetText(CSSStyleSheet &sheet)
  {
    return ToNarrow(sheet.CssText(DefaultSerialisationContext()));
  }

  /// @brief The rule at `index` in `sheet`, downcast to `TRule`.
  template <typename TRule>
  KRYS_NODISCARD TRule &RuleAt(CSSStyleSheet &sheet, uint32 index)
  {
    auto *rule = sheet.Item(index);
    REQUIRE(rule);
    REQUIRE(Is<TRule>(*rule));
    return Downcast<TRule>(*rule);
  }

  /// @brief The exception code `result` holds; it must hold one.
  template <typename T>
  KRYS_NODISCARD ExceptionCode ExceptionCodeOf(const ExceptionOr<T> &result)
  {
    REQUIRE(result.HasException());
    return result.GetException().Code();
  }
}
