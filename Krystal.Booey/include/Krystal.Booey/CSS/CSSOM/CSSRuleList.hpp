#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::css
{
  class CSSRule;
  class CSSStyleSheet;

  /// @brief An ordered list of rules exposed to scripts.
  /// @see https://drafts.csswg.org/cssom/#the-cssrulelist-interface
  class CSSRuleList : public AbstractRefCounted
  {
    KRYS_NON_COPYABLE(CSSRuleList);

  protected:
    CSSRuleList() noexcept;

  public:
    virtual ~CSSRuleList() noexcept;

    KRYS_NODISCARD virtual uint32 Length() const noexcept = 0;
    KRYS_NODISCARD virtual CSSRule *Item(uint32 index) const noexcept = 0;

    KRYS_NODISCARD bool IsSupportedPropertyIndex(uint32 index) const noexcept
    {
      return Item(index);
    }

    KRYS_NODISCARD virtual CSSStyleSheet *GetStyleSheet() const noexcept = 0;
  };

  /// @brief A rule list that owns its rules.
  class StaticCSSRuleList final : public CSSRuleList, public RefCounted<StaticCSSRuleList>
  {
  private:
    List<RefPtr<CSSRule>> _rules;

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

    KRYS_NODISCARD uint32 GetRefCount() const noexcept final
    {
      return RefCounted::GetRefCount();
    }

    KRYS_NODISCARD static Ref<StaticCSSRuleList> Create() noexcept;

    KRYS_NODISCARD List<RefPtr<CSSRule>> &Rules() noexcept
    {
      return _rules;
    }

    KRYS_NODISCARD CSSStyleSheet *GetStyleSheet() const noexcept final
    {
      return nullptr;
    }

  private:
    KRYS_NODISCARD uint32 Length() const noexcept final
    {
      return static_cast<uint32>(_rules.size());
    }

    KRYS_NODISCARD CSSRule *Item(uint32 index) const noexcept final
    {
      return index < _rules.size() ? _rules[index].get() : nullptr;
    }
  };

  /// @brief A rule list that reads a rule's child rules as they are now. The rule owns the live list.
  template <typename TRule>
  class LiveCSSRuleList final : public CSSRuleList
  {
  private:
    TRule &_rule;

  public:
    explicit LiveCSSRuleList(TRule &rule) noexcept : _rule(rule)
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

    KRYS_NODISCARD uint32 GetRefCount() const noexcept final
    {
      return _rule.GetRefCount();
    }

  private:
    KRYS_NODISCARD uint32 Length() const noexcept final
    {
      return _rule.Length();
    }

    KRYS_NODISCARD CSSRule *Item(uint32 index) const noexcept final
    {
      return _rule.Item(index);
    }

    KRYS_NODISCARD CSSStyleSheet *GetStyleSheet() const noexcept final
    {
      return _rule.ParentStyleSheet();
    }
  };
}
