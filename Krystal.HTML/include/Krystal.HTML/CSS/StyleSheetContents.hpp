#pragma once

#include "Krystal.HTML/CSS/Types/CSSOMString.hpp"
#include "Krystal.Lib/Mixins/RefCounted.hpp"
#include "Krystal.Lib/Pointers/RefPtr.hpp"
#include "Krystal.Lib/Types/Map.hpp"
#include "Krystal.Lib/Types/Maybe.hpp"
#include "Krystal.Lib/Types/SmallList.hpp"

namespace Krys::HTML
{
  class CSSImportRule;
  class CSSNamespaceRule;
  class CSSRule;

  class StyleSheetContents : public RefCounted<StyleSheetContents>
  {
    using PrefixNamespaceURIMap = Map<CSSOMStringAtom, CSSOMStringAtom>;

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

    KRYS_NODISCARD RawPtr<CSSRule> RuleAt(size_t index) const noexcept;

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