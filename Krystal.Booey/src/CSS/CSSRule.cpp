#include "Krystal.Booey/CSS/CSSOM/CSSRule.hpp"
#include "Krystal.Booey/CSS/CSSStyleSheet.hpp"
#include "Krystal.Booey/CSS/MediaList.hpp"
#include "Krystal.Booey/CSS/CSSOM/CSSImportRule.hpp"
#include "Krystal.Booey/CSS/CSSOM/CSSNamespaceRule.hpp"
#include "Krystal.Booey/CSS/CSSOM/CSSRuleList.hpp"
#include "Krystal.Booey/CSS/CSSOM/CSSStyleRule.hpp"
#include "Krystal.Booey/CSS/StyleSheetContents.hpp"

namespace krys::boo::css
{
  CSSRule::CSSRule(css::RuleType type, CSSStyleSheet *stylesheet) noexcept
      : _parentRule(nullptr), _type(type), _parentStylesheet(ShareCheckedPtr(stylesheet))
  {
  }

#pragma region CSSRule

  dom::ExceptionOr<void> CSSRule::CssText(const CSSOMString &) noexcept
  {
    return {};
  }

  RefPtr<CSSRule> CSSRule::ParentRule() const noexcept
  {
    return _parentRule;
  }

  RefPtr<CSSStyleSheet> CSSRule::ParentStyleSheet() const noexcept
  {
    return _parentStylesheet;
  }

  uint8 CSSRule::Type() const noexcept
  {
    switch (RuleType())
    {
      case RuleType::Style:
      {
        return 1u;
      }
      case RuleType::Import:
      {
        return 3u;
      }
      case RuleType::Media:
      {
        return 4u;
      }
      case RuleType::FontFace:
      {
        return 5u;
      }
      case RuleType::Page:
      {
        return 6u;
      }
      case RuleType::Keyframes:
      {
        return 7u;
      }
      case RuleType::Keyframe:
      {
        return 8u;
      }
      case RuleType::Margin:
      {
        return 9u;
      }
      case RuleType::Namespace:
      {
        return 10u;
      }
      case RuleType::CounterStyle:
      {
        return 11u;
      }
      case RuleType::Supports:
      {
        return 12u;
      }
      case RuleType::FontFeatureValues:
      {
        return 14u;
      }
      case RuleType::Viewport:
      {
        return 15u;
      }
    }

    return 0u;
  }

#pragma endregion
}