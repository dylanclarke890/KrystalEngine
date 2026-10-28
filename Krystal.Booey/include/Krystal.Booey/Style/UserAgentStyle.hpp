#pragma once

#include "Krystal.Core/Numeric.hpp"

namespace krys::boo::css
{
  class StyleSheetContents;
}

namespace krys::boo::dom
{
  class Element;
}

namespace krys::boo::style
{
  class RuleSet;

  /// @brief The user agent style sheets and the rule sets built from them, shared by every document. Sheets
  /// for SVG, MathML and some HTML features are added when the first element that needs them is styled.
  class UserAgentStyle
  {
  public:
    static RuleSet *DefaultStyle;
    static RuleSet *DefaultQuirksStyle;
    static RuleSet *DefaultPrintStyle;
    static uint32 DefaultStyleVersion;

    static css::StyleSheetContents *DefaultStyleSheet;
    static css::StyleSheetContents *QuirksStyleSheet;
    static css::StyleSheetContents *SVGStyleSheet;
    static css::StyleSheetContents *MathMLStyleSheet;
    static css::StyleSheetContents *MathMLCoreExtrasStyleSheet;
    static css::StyleSheetContents *MathMLFontSizeMathStyleSheet;
    static css::StyleSheetContents *MediaQueryStyleSheet;
    static css::StyleSheetContents *HorizontalFormControlsStyleSheet;
    static css::StyleSheetContents *PopoverStyleSheet;
    static css::StyleSheetContents *CounterStylesStyleSheet;
    static css::StyleSheetContents *ViewTransitionsStyleSheet;

    static void InitDefaultStyleSheet() noexcept;
    static void EnsureDefaultStyleSheetsForElement(const dom::Element &element) noexcept;

  private:
    static void AddToDefaultStyle(css::StyleSheetContents &sheet) noexcept;
  };
}
