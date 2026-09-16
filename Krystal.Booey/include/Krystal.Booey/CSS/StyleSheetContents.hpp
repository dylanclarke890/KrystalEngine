#pragma once

#include "Krystal.Booey/CSS/Types/CSSOMString.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Types/HashMap.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/SmallList.hpp"

namespace krys::boo::css
{
  class CSSImportRule;
  class CSSNamespaceRule;
  class CSSRule;

  class StyleSheetContents : public RefCounted<StyleSheetContents>
  {
    using PrefixNamespaceURIMap = HashMap<CSSOMStringAtom, CSSOMStringAtom>;

  private:
    SmallList<Ref<CSSImportRule>> _importRules;
    SmallList<Ref<CSSNamespaceRule>> _namespaceRules;
    SmallList<Ref<CSSRule>> _childRules;
    PrefixNamespaceURIMap _namespaces;
    CSSOMStringAtom _defaultNamespace;

  public:
    KRYS_NODISCARD bool IsEmpty() const noexcept
    {
      return TotalRules() == 0uz;
    }

    KRYS_NODISCARD size_t TotalRules() const noexcept;

    KRYS_NODISCARD CSSRule *RuleAt(size_t index) const noexcept;

    bool InsertRule(Ref<CSSRule> &&rule, size_t index) noexcept;

    bool DeleteRule(size_t index) noexcept;

#pragma region Namespaces

    void AddNamespace(const CSSOMStringAtom &prefix, const CSSOMStringAtom &namespaceURI) noexcept;

    KRYS_NODISCARD const CSSOMStringAtom &DefaultNamespace() const noexcept
    {
      return _defaultNamespace;
    }

    KRYS_NODISCARD const CSSOMStringAtom &NamespaceForPrefix(const CSSOMStringAtom &prefix) const noexcept;

#pragma endregion
  };
}