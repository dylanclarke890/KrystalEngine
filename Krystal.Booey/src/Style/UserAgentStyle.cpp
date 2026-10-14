#include "Krystal.Booey/Style/UserAgentStyle.hpp"
#include "Krystal.Booey/CSS/Counter/CounterStyleRegistry.hpp"
#include "Krystal.Booey/CSS/Parser/ParserContext.hpp"
#include "Krystal.Booey/CSS/Queries/MediaQueryEvaluator.hpp"
#include "Krystal.Booey/CSS/Rules/CounterStyleRule.hpp"
#include "Krystal.Booey/CSS/Rules/KeyframesRule.hpp"
#include "Krystal.Booey/CSS/StyleSheetContents.hpp"
#include "Krystal.Booey/CSS/UserAgentStyleSheets.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/HTML/HTMLElement.hpp"
#include "Krystal.Booey/MathML/MathMLElement.hpp"
#include "Krystal.Booey/Style/Resolver.hpp"
#include "Krystal.Booey/Style/RuleSet.hpp"
#include "Krystal.Booey/Style/RuleSetBuilder.hpp"
#include "Krystal.Booey/SVG/SVGElement.hpp"
#include "Krystal.Core/TypeCast.hpp"
#include "Krystal.Core/Utils/NeverDestroyed.hpp"

namespace krys::boo::style
{
  RuleSet *UserAgentStyle::DefaultStyle;
  RuleSet *UserAgentStyle::DefaultQuirksStyle;
  RuleSet *UserAgentStyle::DefaultPrintStyle;
  uint32 UserAgentStyle::DefaultStyleVersion;

  css::StyleSheetContents *UserAgentStyle::DefaultStyleSheet;
  css::StyleSheetContents *UserAgentStyle::QuirksStyleSheet;
  css::StyleSheetContents *UserAgentStyle::SVGStyleSheet;
  css::StyleSheetContents *UserAgentStyle::MathMLStyleSheet;
  css::StyleSheetContents *UserAgentStyle::MathMLLegacyFontSizeMathStyleSheet;
  css::StyleSheetContents *UserAgentStyle::MediaQueryStyleSheet;
  css::StyleSheetContents *UserAgentStyle::PopoverStyleSheet;
  css::StyleSheetContents *UserAgentStyle::HorizontalFormControlsStyleSheet;
  css::StyleSheetContents *UserAgentStyle::CounterStylesStyleSheet;
  css::StyleSheetContents *UserAgentStyle::ViewTransitionsStyleSheet;

  namespace
  {
    KRYS_NODISCARD const css::mq::MediaQueryEvaluator &ScreenEval() noexcept
    {
      static NeverDestroyed<const css::mq::MediaQueryEvaluator> staticScreenEval(
        css::CSSOMStringAtom(u8"screen"));
      return staticScreenEval;
    }

    KRYS_NODISCARD const css::mq::MediaQueryEvaluator &PrintEval() noexcept
    {
      static NeverDestroyed<const css::mq::MediaQueryEvaluator> staticPrintEval(
        css::CSSOMStringAtom(u8"print"));
      return staticPrintEval;
    }

    KRYS_NODISCARD css::StyleSheetContents *ParseUASheet(css::CSSOMStringView string) noexcept
    {
      // leak the sheet on purpose
      css::StyleSheetContents &sheet =
        *css::StyleSheetContents::Create(css::ParserContext(css::ParserMode::UASheet)).release();
      sheet.ParseString(css::CSSOMString(string));
      return &sheet;
    }

    void AddToCounterStyleRegistry(css::StyleSheetContents &sheet) noexcept
    {
      for (auto &rule : sheet.ChildRules())
      {
        if (auto *counterStyleRule = DynamicDowncast<css::CounterStyleRule>(*rule))
        {
          css::CounterStyleRegistry::AddUserAgentCounterStyle(counterStyleRule->Descriptors());
        }
      }
      css::CounterStyleRegistry::ResolveUserAgentReferences();
    }

    void AddUserAgentKeyframes(css::StyleSheetContents &sheet) noexcept
    {
      // This does not handle nested rules.
      for (auto &rule : sheet.ChildRules())
      {
        if (auto *styleRuleKeyframes = DynamicDowncast<css::KeyframesRule>(*rule))
        {
          Resolver::AddUserAgentKeyframeStyle(ShareRef(*styleRuleKeyframes));
        }
      }
    }

    /// @brief Whether @p element is a form control (button, fieldset, input, output, select or textarea), a
    /// meter or a progress element.
    KRYS_NODISCARD bool IsFormControlMeterOrProgress(const dom::Element &element) noexcept
    {
      switch (element.TagQualifiedName().TagName())
      {
        case html::TagName::button:
        case html::TagName::fieldset:
        case html::TagName::input:
        case html::TagName::output:
        case html::TagName::select:
        case html::TagName::textarea:
        case html::TagName::meter:
        case html::TagName::progress: return true;
        default:                      return false;
      }
    }
  }

  void UserAgentStyle::AddToDefaultStyle(css::StyleSheetContents &sheet) noexcept
  {
    {
      RuleSetBuilder screenBuilder(*DefaultStyle, ScreenEval());
      screenBuilder.AddRulesFromSheet(sheet);
    }

    {
      RuleSetBuilder printBuilder(*DefaultPrintStyle, PrintEval());
      printBuilder.AddRulesFromSheet(sheet);
    }

    // Build a stylesheet consisting of non-trivial media queries seen in default style.
    // Rulesets for these can't be global and need to be built in document context.
    for (auto &rule : sheet.ChildRules())
    {
      auto *mediaRule = DynamicDowncast<css::MediaRule>(*rule);
      if (!mediaRule)
      {
        continue;
      }
      auto &mediaQuery = mediaRule->MediaQueries();
      if (ScreenEval().Evaluate(mediaQuery))
      {
        continue;
      }
      if (PrintEval().Evaluate(mediaQuery))
      {
        continue;
      }
      MediaQueryStyleSheet->ParserAppendRule(mediaRule->Copy());
    }

    ++DefaultStyleVersion;
  }

  void UserAgentStyle::InitDefaultStyleSheet() noexcept
  {
    if (DefaultStyle)
    {
      return;
    }

    DefaultStyle = RuleSet::Create().release();
    DefaultPrintStyle = RuleSet::Create().release();
    DefaultQuirksStyle = RuleSet::Create().release();
    MediaQueryStyleSheet =
      css::StyleSheetContents::Create(css::ParserContext(css::ParserMode::UASheet)).release();

    // FIXME: Append the render theme's extra default style sheet, once there is a render theme.
    DefaultStyleSheet = ParseUASheet(css::HTMLUserAgentStyleSheet);
    AddToDefaultStyle(*DefaultStyleSheet);

    CounterStylesStyleSheet = ParseUASheet(css::CounterStylesUserAgentStyleSheet);
    AddToCounterStyleRegistry(*CounterStylesStyleSheet);

    QuirksStyleSheet = ParseUASheet(css::QuirksUserAgentStyleSheet);

    {
      RuleSetBuilder quirkBuilder(*DefaultQuirksStyle, ScreenEval());
      quirkBuilder.AddRulesFromSheet(*QuirksStyleSheet);
    }

    ++DefaultStyleVersion;
  }

  void UserAgentStyle::EnsureDefaultStyleSheetsForElement(const dom::Element &element) noexcept
  {
    if (Is<html::HTMLElement>(element))
    {
      // FIXME: The switch control style sheet, for checkboxes with the switch attribute, once there are
      // settings (SwitchControlEnabled is off by default).

      if (!PopoverStyleSheet && element.HasAttribute(dom::DOMStringAtom(u8"popover")))
      {
        PopoverStyleSheet = ParseUASheet(css::PopoverUserAgentStyleSheet);
        AddToDefaultStyle(*PopoverStyleSheet);
      }

      // Vertical form controls are off by default (the VerticalFormControlsEnabled setting).
      if (IsFormControlMeterOrProgress(element))
      {
        if (!HorizontalFormControlsStyleSheet)
        {
          HorizontalFormControlsStyleSheet = ParseUASheet(css::HorizontalFormControlsUserAgentStyleSheet);
          AddToDefaultStyle(*HorizontalFormControlsStyleSheet);
        }
      }
    }
    else if (Is<svg::SVGElement>(element))
    {
      if (!SVGStyleSheet)
      {
        SVGStyleSheet = ParseUASheet(css::SVGUserAgentStyleSheet);
        AddToDefaultStyle(*SVGStyleSheet);
      }
    }
    else if (Is<mathml::MathMLElement>(element))
    {
      if (!MathMLStyleSheet)
      {
        MathMLStyleSheet = ParseUASheet(css::MathMLUserAgentStyleSheet);
        AddToDefaultStyle(*MathMLStyleSheet);
      }

      // FIXME: The MathML Core extras and font-size: math style sheets, once there are settings
      // (CoreMathMLEnabled and CSSMathDepthEnabled are off by default).
      if (!MathMLLegacyFontSizeMathStyleSheet)
      {
        MathMLLegacyFontSizeMathStyleSheet = ParseUASheet(css::MathMLLegacyFontSizeMathUserAgentStyleSheet);
        AddToDefaultStyle(*MathMLLegacyFontSizeMathStyleSheet);
      }
    }

    // FIXME: The fullscreen style sheet, once documents have the Fullscreen API.

    if (!ViewTransitionsStyleSheet)
    {
      ViewTransitionsStyleSheet = ParseUASheet(css::ViewTransitionsUserAgentStyleSheet);
      AddToDefaultStyle(*ViewTransitionsStyleSheet);
      AddUserAgentKeyframes(*ViewTransitionsStyleSheet);
    }

    krys_debug_assert(DefaultStyle->Features().IdsInRules.empty());
  }
}
