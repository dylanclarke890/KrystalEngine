#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Containers/List.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Memory/RefCounted.hpp"
#include "Krystal.Core/Memory/RefPtr.hpp"

namespace krys::boo
{
  class CSSRule;
  class CSSStyleSheet;

  /// @brief An ordered collection of rules, as scripts see it.
  /// @see https://drafts.csswg.org/cssom/#the-cssrulelist-interface
  class CSSRuleList : public AbstractRefCounted
  {
    KRYS_NON_COPYABLE(CSSRuleList);

  protected:
    CSSRuleList() noexcept;

  public:
    ~CSSRuleList() noexcept override;

#pragma region CSSRuleList - https://drafts.csswg.org/cssom/#the-cssrulelist-interface

    /// @brief The rule at @p index, or null past the end.
    /// @see https://drafts.csswg.org/cssom/#dom-cssrulelist-item
    KRYS_NODISCARD virtual CSSRule *Item(uint32 index) const noexcept = 0;

    /// @brief The number of rules.
    /// @see https://drafts.csswg.org/cssom/#dom-cssrulelist-length
    KRYS_NODISCARD virtual uint32 Length() const noexcept = 0;

#pragma endregion

    KRYS_NODISCARD bool IsSupportedPropertyIndex(uint32 index) const noexcept
    {
      return index < Length();
    }

    /// @brief The style sheet the rules are in, if the list follows one.
    KRYS_NODISCARD virtual CSSStyleSheet *GetStyleSheet() const noexcept = 0;
  };

  /// @brief A list of rules fixed when it's made, which keeps them alive.
  class StaticCSSRuleList final : public CSSRuleList, public RefCounted<StaticCSSRuleList>
  {
  private:
    List<RefPtr<CSSRule>> _rules;

    StaticCSSRuleList() noexcept;

  public:
    ~StaticCSSRuleList() noexcept override;

    KRYS_NODISCARD static Ref<StaticCSSRuleList> Create() noexcept;

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

    /// @brief The rules, to fill the list with.
    KRYS_NODISCARD List<RefPtr<CSSRule>> &Rules() noexcept
    {
      return _rules;
    }

    KRYS_NODISCARD CSSRule *Item(uint32 index) const noexcept final
    {
      return index < _rules.size() ? _rules[index].get() : nullptr;
    }

    KRYS_NODISCARD uint32 Length() const noexcept final
    {
      return static_cast<uint32>(_rules.size());
    }

    KRYS_NODISCARD CSSStyleSheet *GetStyleSheet() const noexcept final
    {
      return nullptr;
    }
  };

  /// @brief The list of a sheet's or rule's child rules, which reads them as they are when asked. Its owner
  /// holds it and lends it its reference count.
  template <typename TOwner>
  class LiveCSSRuleList final : public CSSRuleList
  {
  private:
    TOwner &_owner;

  public:
    explicit LiveCSSRuleList(TOwner &owner) noexcept : _owner(owner)
    {
    }

    void AddRef() const noexcept final
    {
      _owner.AddRef();
    }

    void SubRef() const noexcept final
    {
      _owner.SubRef();
    }

    KRYS_NODISCARD uint32 GetRefCount() const noexcept final
    {
      return _owner.GetRefCount();
    }

    KRYS_NODISCARD CSSRule *Item(uint32 index) const noexcept final
    {
      return _owner.Item(index);
    }

    KRYS_NODISCARD uint32 Length() const noexcept final
    {
      return _owner.Length();
    }

    KRYS_NODISCARD CSSStyleSheet *GetStyleSheet() const noexcept final
    {
      return _owner.ParentStyleSheet();
    }
  };
}
