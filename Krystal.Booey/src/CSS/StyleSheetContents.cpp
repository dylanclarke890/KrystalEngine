#include "Krystal.Booey/CSS/StyleSheetContents.hpp"

namespace krys::boo::css
{
  void StyleSheetContents::ParserAddNamespace(const CSSOMStringAtom &prefix,
                                              const CSSOMStringAtom &uri) noexcept
  {
    krys_debug_assert(uri != CSSOMStringAtom::Null());
    if (prefix == CSSOMStringAtom::Null())
    {
      _defaultNamespace = uri;
      return;
    }
    _namespaces.insert_or_assign(prefix, uri);
  }

  const CSSOMStringAtom &
    StyleSheetContents::NamespaceURIFromPrefix(const CSSOMStringAtom &prefix) const noexcept
  {
    auto it = _namespaces.find(prefix);
    if (it == _namespaces.end())
    {
      return CSSOMStringAtom::Null();
    }
    return it->second;
  }
}
