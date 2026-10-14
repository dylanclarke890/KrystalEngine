#pragma once

#include "Krystal.Booey/CSS/Types/CSSOMString.hpp"
#include "Krystal.Booey/DOM/Types/DOMString.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Types/HashMap.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::css
{
  /// @brief The parsed contents of a style sheet, shared between the CSSStyleSheet objects that use it.
  // FIXME: Only the namespaces so far; the rules, import handling and parsing come with the rule model.
  class StyleSheetContents : public RefCounted<StyleSheetContents>
  {
    using PrefixNamespaceURIMap = HashMap<CSSOMStringAtom, CSSOMStringAtom>;

  private:
    PrefixNamespaceURIMap _namespaces;
    CSSOMStringAtom _defaultNamespace {dom::StarAtom()};

  public:
    StyleSheetContents() noexcept = default;

    KRYS_NODISCARD static Ref<StyleSheetContents> Create() noexcept
    {
      return CreateRef<StyleSheetContents>();
    }

    KRYS_NODISCARD const CSSOMStringAtom &DefaultNamespace() const noexcept
    {
      return _defaultNamespace;
    }

    KRYS_NODISCARD const CSSOMStringAtom &
      NamespaceURIFromPrefix(const CSSOMStringAtom &prefix) const noexcept;

    void ParserAddNamespace(const CSSOMStringAtom &prefix, const CSSOMStringAtom &uri) noexcept;
  };
}
