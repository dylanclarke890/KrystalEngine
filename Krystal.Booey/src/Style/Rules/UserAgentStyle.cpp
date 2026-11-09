/*
 * Copyright (C) 1999 Lars Knoll (knoll@kde.org)
 *           (C) 2004-2005 Allan Sandfeld Jensen (kde@carewolf.com)
 * Copyright (C) 2006, 2007 Nicholas Shanks (webkit@nickshanks.com)
 * Copyright (C) 2005-2025 Apple Inc. All rights reserved.
 * Copyright (C) 2007 Alexey Proskuryakov <ap@webkit.org>
 * Copyright (C) 2007, 2008 Eric Seidel <eric@webkit.org>
 * Copyright (C) 2008, 2009 Torch Mobile Inc. All rights reserved. (http://www.torchmobile.com/)
 * Copyright (c) 2011, Code Aurora Forum. All rights reserved.
 * Copyright (C) Research In Motion Limited 2011. All rights reserved.
 * Copyright (C) 2012 Google Inc. All rights reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "Krystal.Booey/Style/Rules/UserAgentStyle.hpp"
#include "Krystal.Booey/CSS/Counter/CounterStyleRegistry.hpp"
#include "Krystal.Booey/CSS/Parser/ParserContext.hpp"
#include "Krystal.Booey/CSS/Properties/StylePropertiesInlines.hpp"
#include "Krystal.Booey/CSS/Queries/MediaQueryEvaluator.hpp"
#include "Krystal.Booey/CSS/Rules/CounterStyleRule.hpp"
#include "Krystal.Booey/CSS/Rules/KeyframesRule.hpp"
#include "Krystal.Booey/CSS/StyleSheets/StyleSheetContents.hpp"
#include "Krystal.Booey/CSS/UserAgentStyleSheets.hpp"
#include "Krystal.Booey/DOM/Nodes/Element.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLElement.hpp"
#include "Krystal.Booey/MathML/Elements/MathMLElement.hpp"
#include "Krystal.Booey/Rendering/RenderTheme.hpp"
#include "Krystal.Booey/Style/Resolver.hpp"
#include "Krystal.Booey/Style/Rules/RuleSet.hpp"
#include "Krystal.Booey/Style/Rules/RuleSetBuilder.hpp"
#include "Krystal.Booey/SVG/Elements/SVGElement.hpp"
#include "Krystal.Core/Threading/NeverDestroyed.hpp"
#include "Krystal.Core/TypeCast.hpp"

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
  css::StyleSheetContents *UserAgentStyle::MathMLCoreExtrasStyleSheet;
  css::StyleSheetContents *UserAgentStyle::MathMLFontSizeMathStyleSheet;
  css::StyleSheetContents *UserAgentStyle::MediaQueryStyleSheet;
  css::StyleSheetContents *UserAgentStyle::PopoverStyleSheet;
  css::StyleSheetContents *UserAgentStyle::HorizontalFormControlsStyleSheet;
  css::StyleSheetContents *UserAgentStyle::CounterStylesStyleSheet;

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

    auto extraDefaultStyleSheet = RenderTheme::Singleton().ExtraDefaultStyleSheet();
    if (extraDefaultStyleSheet.empty())
    {
      DefaultStyleSheet = ParseUASheet(css::HTMLUserAgentStyleSheet);
    }
    else
    {
      DefaultStyleSheet =
        ParseUASheet(utf8_string(css::HTMLUserAgentStyleSheet) + utf8_string(extraDefaultStyleSheet));
    }
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

      if (!MathMLCoreExtrasStyleSheet)
      {
        MathMLCoreExtrasStyleSheet = ParseUASheet(css::MathMLCoreExtrasUserAgentStyleSheet);
        AddToDefaultStyle(*MathMLCoreExtrasStyleSheet);
      }
      if (!MathMLFontSizeMathStyleSheet)
      {
        MathMLFontSizeMathStyleSheet = ParseUASheet(css::MathMLFontSizeMathUserAgentStyleSheet);
        AddToDefaultStyle(*MathMLFontSizeMathStyleSheet);
      }
    }

    // FIXME: The fullscreen style sheet, once documents have the Fullscreen API.

    krys_debug_assert(DefaultStyle->Features().IdsInRules.empty());
  }
}
