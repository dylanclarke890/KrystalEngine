#pragma once

#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/SmallList.hpp"

namespace krys::boo::css
{
  class CSSRule;
  class CSSStyleSheet;

  class CSSRuleList : public AbstractRefCounted
  {
    KRYS_NON_COPYABLE(CSSRuleList);

  protected:
    CSSRuleList() noexcept;

  public:
    virtual ~CSSRuleList() noexcept;

    KRYS_NODISCARD virtual size_t Length() const noexcept = 0;

    KRYS_NODISCARD virtual CSSRule *Item(size_t index) const noexcept = 0;

    KRYS_NODISCARD bool IsSupportedPropertyIndex(size_t index) const noexcept
    {
      return Item(index);
    }

    KRYS_NODISCARD virtual CSSStyleSheet *StyleSheet() const noexcept = 0;
  };

  class StaticCSSRuleList final : public CSSRuleList, public RefCounted<StaticCSSRuleList>
  {
  private:
    SmallList<RefPtr<CSSRule>> _rules;

    StaticCSSRuleList() noexcept;

  public:
    ~StaticCSSRuleList() noexcept;

    void AddRef() const noexcept final
    {
      RefCounted::AddRef();
    }

    void SubRef() const noexcept final
    {
      RefCounted::SubRef();
    }

    KRYS_NODISCARD static Ref<StaticCSSRuleList> Create() noexcept
    {
      return AdoptRef(*new StaticCSSRuleList);
    }

    KRYS_NODISCARD SmallList<RefPtr<CSSRule>> &Rules() noexcept
    {
      return _rules;
    }

    KRYS_NODISCARD CSSStyleSheet *StyleSheet() const noexcept final
    {
      return nullptr;
    }

  private:
    KRYS_NODISCARD size_t Length() const noexcept final
    {
      return _rules.size();
    }

    KRYS_NODISCARD CSSRule *Item(unsigned index) const noexcept final
    {
      return index < _rules.size() ? _rules[index].get() : nullptr;
    }
  };

  template <typename Rule>
  class LiveCSSRuleList final : public CSSRuleList
  {
  private:
    // The rule owns the live list.
    Rule &_rule;

  public:
    LiveCSSRuleList(Rule &rule) noexcept : _rule(rule)
    {
    }

    void AddRef() const noexcept final
    {
      _rule.AddRef();
    }

    void SubRef() const noexcept final
    {
      _rule.SubRef();
    }

  private:
    KRYS_NODISCARD size_t Length() const noexcept final
    {
      return _rule.Length();
    }

    KRYS_NODISCARD CSSRule *Item(size_t index) const noexcept final
    {
      return _rule.Item(index);
    }

    KRYS_NODISCARD CSSStyleSheet *StyleSheet() const noexcept final
    {
      return _rule.StyleSheet();
    }
  };
}
