#include "Krystal.HTML/CSS/CSSRule.hpp"
#include "Krystal.HTML/CSS/CSSRuleList.hpp"
#include "Krystal.HTML/CSS/CSSStyleSheet.hpp"
#include "Krystal.HTML/CSS/MediaList.hpp"

namespace Krys::HTML
{
  CSSRule::CSSRule(CSSRuleType type, RawPtr<CSSStyleSheet> stylesheet) noexcept
      : _parentRule(nullptr), _type(type), _parentStylesheet(ShareCheckedPtr(stylesheet))
  {
  }

#pragma region CSSRule

  ExceptionOr<void> CSSRule::CssText(const CSSOMString &) noexcept
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
      case CSSRuleType::Style:
      {
        return 1u;
      }
      case CSSRuleType::Import:
      {
        return 3u;
      }
      case CSSRuleType::Media:
      {
        return 4u;
      }
      case CSSRuleType::FontFace:
      {
        return 5u;
      }
      case CSSRuleType::Page:
      {
        return 6u;
      }
      case CSSRuleType::Keyframes:
      {
        return 7u;
      }
      case CSSRuleType::Keyframe:
      {
        return 8u;
      }
      case CSSRuleType::Margin:
      {
        return 9u;
      }
      case CSSRuleType::Namespace:
      {
        return 10u;
      }
      case CSSRuleType::CounterStyle:
      {
        return 11u;
      }
      case CSSRuleType::Supports:
      {
        return 12u;
      }
      case CSSRuleType::FontFeatureValues:
      {
        return 14u;
      }
      case CSSRuleType::Viewport:
      {
        return 15u;
      }
    }

    return 0u;
  }

#pragma endregion
}