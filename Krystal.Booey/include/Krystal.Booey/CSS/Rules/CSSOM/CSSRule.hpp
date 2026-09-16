#pragma once

#include <WebCore/CSSParserEnum.h>
#include <WebCore/StyleRuleType.h>
#include <wtf/RefCountedAndCanMakeWeakPtr.h>
#include <wtf/TypeCasts.h>

namespace WebCore
{

  class CSSStyleSheet;
  class StyleRuleBase;
  class StyleRule;
  class StyleRuleWithNesting;

  struct CSSParserContext;

  template <typename>
  class ExceptionOr;

  namespace CSS
  {
    struct SerializationContext;
  }

  class CSSRule : public RefCountedAndCanMakeWeakPtr<CSSRule>
  {
  public:
    virtual ~CSSRule() = default;

    WEBCORE_EXPORT unsigned short typeForCSSOM() const;

    virtual StyleRuleType styleRuleType() const = 0;
    virtual bool isGroupingRule() const
    {
      return false;
    }
    virtual String cssText() const = 0;
    virtual String cssText(const CSS::SerializationContext &) const
    {
      return cssText();
    }
    virtual void reattach(StyleRuleBase &) = 0;

    void setParentStyleSheet(CSSStyleSheet *);
    void setParentRule(CSSRule *);
    CSSStyleSheet *parentStyleSheet() const;
    CSSRule *parentRule() const
    {
      return m_parentIsRule ? m_parentRule : nullptr;
    }
    bool hasStyleRuleAncestor() const;
    CSSParserEnum::NestedContext nestedContext() const;
    virtual RefPtr<StyleRuleWithNesting> prepareChildStyleRuleForNesting(StyleRule &);
    virtual void getChildStyleSheets(HashSet<RefPtr<CSSStyleSheet>> &)
    {
    }

    WEBCORE_EXPORT ExceptionOr<void> setCssText(const String &);

  protected:
    explicit CSSRule(CSSStyleSheet *);

    bool hasCachedSelectorText() const
    {
      return m_hasCachedSelectorText;
    }
    void setHasCachedSelectorText(bool hasCachedSelectorText) const
    {
      m_hasCachedSelectorText = hasCachedSelectorText;
    }

    const CSSParserContext &parserContext() const;

  private:
    mutable unsigned char m_hasCachedSelectorText : 1;
    unsigned char m_parentIsRule : 1;
    union
    {
      CSSRule *m_parentRule;
      CSSStyleSheet *m_parentStyleSheet;
    };
  };

  inline CSSRule::CSSRule(CSSStyleSheet *parent)
      : m_hasCachedSelectorText(false), m_parentIsRule(false), m_parentStyleSheet(parent)
  {
  }

  inline void CSSRule::setParentStyleSheet(CSSStyleSheet *styleSheet)
  {
    m_parentIsRule = false;
    m_parentStyleSheet = styleSheet;
  }

  inline void CSSRule::setParentRule(CSSRule *rule)
  {
    m_parentIsRule = true;
    m_parentRule = rule;
  }

  inline CSSStyleSheet *CSSRule::parentStyleSheet() const
  {
    if (m_parentIsRule)
      return m_parentRule ? m_parentRule->parentStyleSheet() : nullptr;
    return m_parentStyleSheet;
  }

} // namespace WebCore

#define SPECIALIZE_TYPE_TRAITS_CSS_RULE(ToValueTypeName, predicate)                                          \
  SPECIALIZE_TYPE_TRAITS_BEGIN(WebCore::ToValueTypeName)                                                     \
  static bool isType(const WebCore::CSSRule &rule)                                                           \
  {                                                                                                          \
    return rule.styleRuleType() == WebCore::predicate;                                                       \
  }                                                                                                          \
  SPECIALIZE_TYPE_TRAITS_END()
