#pragma once

#include "Krystal.Core/Numeric.hpp"

namespace krys::boo::dom
{
  class Element;
}

namespace krys::boo::css
{
  class RuleSet;
  class StyleSheetContents;
}

namespace krys::boo::style
{
  class UserAgentStyle
  {
  public:
    static size_t DefaultStyleVersion;

    static css::RuleSet *DefaultStyle;
    static css::RuleSet *DefaultQuirksStyle;
    static css::RuleSet *DefaultPrintStyle;

    static css::StyleSheetContents *DefaultStyleSheet;
    static css::StyleSheetContents *QuirksStyleSheet;
    static css::StyleSheetContents *SVGStyleSheet;
    static css::StyleSheetContents *MathMLStyleSheet;
    static css::StyleSheetContents *MathMLCoreExtrasStyleSheet;
    static css::StyleSheetContents *MathMLFontSizeMathStyleSheet;
    static css::StyleSheetContents *MathMLLegacyFontSizeMathStyleSheet;
    static css::StyleSheetContents *MediaQueryStyleSheet;
    static css::StyleSheetContents *HorizontalFormControlsStyleSheet;
    static css::StyleSheetContents *HTMLSwitchControlStyleSheet;
    static css::StyleSheetContents *PopoverStyleSheet;
    static css::StyleSheetContents *CounterStylesStyleSheet;
    static css::StyleSheetContents *ViewTransitionsStyleSheet;

    static void InitDefaultStyleSheet() noexcept;
    static void EnsureDefaultStyleSheetsForElement(const dom::Element &element) noexcept;

  private:
    static void AddToDefaultStyle(css::StyleSheetContents &stylesheet) noexcept;
  };
}